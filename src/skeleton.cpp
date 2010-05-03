#include <cstdio>
#include "skeleton.hh"
#include "assert.hh"
#include "lbfloader.hh"
#include "lbfhelpers.hh"

Skeleton::Skeleton(int num_joints)
	: m_num_joints(num_joints)
	, m_root_offset(0,0,0)
	, m_root_rotation(0,0,0,1) // no rotation
	, m_translations(0)
	, m_parents(0)
	, m_full_translations(0)
	, m_full_rotations(0)
{
	m_joint_names = new std::string[num_joints];
	m_translations = new Vec3[num_joints];
	m_parents = new int[num_joints];
	m_full_translations = new Vec3[num_joints];
	m_full_rotations = new Quaternion[num_joints];
	
	memset(m_translations, 0, sizeof(Vec3)*num_joints);
	memset(m_parents, -1, sizeof(int)*num_joints);
	memset(m_full_translations, 0, sizeof(Vec3)*num_joints);
	memset(m_full_rotations, 0, sizeof(Quaternion)*num_joints);
}

Skeleton::~Skeleton()
{
	delete[] m_joint_names;
	delete[] m_translations;
	delete[] m_parents;
	delete[] m_full_translations;
	delete[] m_full_rotations;
}

void Skeleton::ComputeTransforms()
{
	Quaternion root_rotation = GetRootRotation();
	Vec3 root_offset = GetRootOffset();

	const int num_joints = m_num_joints;

	for(int i = 0; i < num_joints; ++i)
	{			
		int parent = GetJointParent(i);
		// local_rot = skelrestRot * anim * invSkelRestRot
		// but anim is identity, so no rotation is applied.
			
		if(parent == -1) { 
			m_full_rotations[i] = root_rotation ;
			m_full_translations[i] = root_offset ;
		} else {
			Vec3 local_offset = GetJointTranslation(parent);

			m_full_rotations[i] = m_full_rotations[parent] ;
			m_full_translations[i] = m_full_translations[parent] + 
				rotate(local_offset, m_full_rotations[parent]);
		}
	}
}

Mat4 Skeleton::GetSkelToJointTransform(int idx) const
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	Mat4 result = conjugate(m_full_rotations[idx]).to_matrix();
	Vec3 offset = transform_vector(result, -m_full_translations[idx]);
	result.m[3] = offset.x; 
	result.m[7] = offset.y;
	result.m[11] = offset.z;
	return result;
}

const Vec3& Skeleton::GetJointToSkelOffset(int idx) const
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_full_translations[idx];
}

const Quaternion& Skeleton::GetJointToSkelRotation(int idx) const
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_full_rotations[idx];
}

Vec3& Skeleton::GetJointTranslation(int idx) 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_translations[idx];
}

const Vec3& Skeleton::GetJointTranslation(int idx) const 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_translations[idx];
}

const char* Skeleton::GetJointName(int idx)  const
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_joint_names[idx].c_str();
}

void Skeleton::SetJointName(int idx, const char* name )
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	m_joint_names[idx] = name;
}

void Skeleton::SetJointParent(int idx, int parent) 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	ASSERT(parent >= -1 && parent < m_num_joints);
	m_parents[idx] = parent;
}

int Skeleton::GetJointParent(int idx) const
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_parents[idx];
}


struct skel_save_info
{
	int num_joints;
	Vec3 root_translation;
	Quaternion root_rotation;
};

LBF::WriteNode* Skeleton::CreateSkeletonWriteNode( ) const
{
	skel_save_info save_info;
	
	save_info.num_joints = this->GetNumJoints();
	save_info.root_translation = this->GetRootOffset();
	save_info.root_rotation = this->GetRootRotation();

	LBF::WriteNode* skelNode = new LBF::WriteNode(LBF::SKELETON,0,sizeof(skel_save_info));
	skelNode->PutData(&save_info, sizeof(save_info));

	const char* name = this->GetName();
	int nameLen = strlen(name);
	LBF::WriteNode* nameNode = new LBF::WriteNode(LBF::SKELETON_NAME,0,nameLen*sizeof(char));
	nameNode->PutData(name, nameLen*sizeof(char));
	skelNode->AddChild(nameNode);
	
	LBF::WriteNode* translationsNode = new LBF::WriteNode(LBF::SKELETON_TRANSLATIONS, 0, (this->GetNumJoints())*sizeof(Vec3));
	skelNode->AddChild(translationsNode);
	translationsNode->PutData(&this->GetJointTranslation(0),sizeof(Vec3)*this->GetNumJoints());

	LBF::WriteNode* parentsNode = new LBF::WriteNode(LBF::SKELETON_PARENTS, 0, (this->GetNumJoints())*sizeof(int));
	skelNode->AddChild(parentsNode);
	parentsNode->PutData(this->m_parents, sizeof(int)*this->GetNumJoints());

	LBF::WriteNode* stringNode = createStdStringTableNode(LBF::SKELETON_NAMES, 0, m_joint_names, this->GetNumJoints());
	skelNode->AddChild(stringNode);

	return skelNode;
}

static const int kSaneJointCount = 5000;

Skeleton* Skeleton::CreateSkeletonFromReadNode( const LBF::ReadNode& rn )
{
	if(!rn.Valid())
		return 0;

	BufferReader reader;

	skel_save_info info;
	reader = rn.GetReader();
	reader.Get(&info, sizeof(info));

	if(info.num_joints > kSaneJointCount) {
		fprintf(stderr, "Woah, crazy. Sanity check failed, a skeleton with %d joints probably isn't what you wanted (found %d).\n", kSaneJointCount, info.num_joints);
		return 0;
	}

	Skeleton* skel = new Skeleton(info.num_joints);
	skel->SetRootOffset(info.root_translation);
	skel->SetRootRotation(info.root_rotation);

	LBF::ReadNode rnName = rn.GetFirstChild(LBF::SKELETON_NAME);
	if(rnName.Valid()) {
		reader = rnName.GetReader();
		int len = rnName.GetNodeDataLength() + 1;
		std::string tempStr(len+1, '\0');
		reader.Get(&tempStr[0], rnName.GetNodeDataLength());
		skel->SetName(tempStr);
	}

	LBF::ReadNode rnTranslations = rn.GetFirstChild(LBF::SKELETON_TRANSLATIONS);
	if(rnTranslations.Valid()) {
		reader = rnTranslations.GetReader();
		for(int i = 0; i < info.num_joints; ++i) {
			reader.Get(&skel->GetJointTranslation(i), sizeof(Vec3));
		}
	}

	LBF::ReadNode rnParents = rn.GetFirstChild(LBF::SKELETON_PARENTS);
	if(rnParents.Valid()) {
		reader = rnParents.GetReader();
		for(int i = 0; i < info.num_joints; ++i) {
			int parent = -1;
			reader.Get(&parent,sizeof(int));
			skel->SetJointParent(i, parent);
		}
	}

	LBF::ReadNode rnJoints = rn.GetFirstChild(LBF::SKELETON_NAMES);
	if(rnJoints.Valid()) {
		readStdStringTable(rnJoints, skel->m_joint_names, info.num_joints);
	}

	skel->ComputeTransforms();
	return skel;
}

////////////////////////////////////////////////////////////////////////////////
SkeletonWeights::SkeletonWeights(int num_joints)
	: m_num_joints(num_joints)
	, m_weights(0)
{
	m_weights = new float[num_joints];
	for(int i = 0; i < num_joints; ++i) m_weights[i] = 1.f;
}

SkeletonWeights::~SkeletonWeights()
{
	delete[] m_weights;
}

void SkeletonWeights::SetJointWeight(int idx, float weight) 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	m_weights[idx] = weight;
}

float SkeletonWeights::GetJointWeight(int idx) const 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_weights[idx];
}

LBF::WriteNode* SkeletonWeights::CreateWriteNode() const 
{
	LBF::WriteNode* weightsNode = new LBF::WriteNode(LBF::SKELETON_JOINT_WEIGHTS, 0, m_num_joints*sizeof(float));
	weightsNode->PutData(this->m_weights, sizeof(float)*m_num_joints);
	return weightsNode;
}

SkeletonWeights* SkeletonWeights::CreateFromReadNode( const LBF::ReadNode& rn )
{
	if(!rn.Valid()) return 0;

	int num_joints = rn.GetNodeDataLength() / sizeof(float);
	SkeletonWeights* result = new SkeletonWeights(num_joints);
	rn.GetData(result->m_weights, sizeof(float)*num_joints);
	return result;
}




