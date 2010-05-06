#include <cstdio>
#include "skeleton.hh"
#include "assert.hh"
#include "lbfloader.hh"
#include "lbfhelpers.hh"
#include "sql/sqlite3.h"

Skeleton::Skeleton(sqlite3* db, sqlite3_int64  id)
	: m_db(db)
	, m_id(id)
	, m_num_joints(0)
	, m_joint_names(0)
	, m_root_offset(0,0,0)
	, m_root_rotation(0,0,0,1) // no rotation
	, m_translations(0)
	, m_parents(0)
	, m_full_translations(0)
	, m_full_rotations(0)
	, m_weights(db, id)
{
	if(!LoadFromDB()) {
		m_id = 0;
	}
}

Skeleton::~Skeleton()
{
	delete[] m_joint_names;
	delete[] m_translations;
	delete[] m_parents;
	delete[] m_full_translations;
	delete[] m_full_rotations;
}

bool Skeleton::LoadFromDB()
{
	// load basic skeleton info
	Query q_info(m_db, "SELECT name,"
				 "root_offset_x,root_offset_y,root_offset_z,"
				 "root_rotation_a,root_rotation_b,root_rotation_c,root_rotation_r "
				 "FROM skeleton WHERE id = ?");
	q_info.BindInt64( 1, m_id );
	if( q_info.Step() ) {
		m_name = q_info.ColText(0);
		m_root_offset = q_info.ColVec3(1);
		m_root_rotation = q_info.ColQuaternion(4);
	} 

	if(q_info.IsError()) return false;

	Query q_count_joints(m_db, 
						 "SELECT COUNT(*) FROM skeleton_joints WHERE skel_id = ?");
	q_count_joints.BindInt64(1, m_id);
	if(q_count_joints.Step()) {
		m_num_joints = q_count_joints.ColInt(0);
	}
	
	if(q_count_joints.IsError()) return false;

	m_joint_names = new std::string[m_num_joints];
	m_translations = new Vec3[m_num_joints];
	m_parents = new int[m_num_joints];
	m_full_translations = new Vec3[m_num_joints];
	m_full_rotations = new Quaternion[m_num_joints];
	
	memset(m_translations, 0, sizeof(Vec3)*m_num_joints);
	memset(m_parents, -1, sizeof(int)*m_num_joints);
	memset(m_full_translations, 0, sizeof(Vec3)*m_num_joints);
	memset(m_full_rotations, 0, sizeof(Quaternion)*m_num_joints);
	
	Query q_get_joints(m_db, 
					   "SELECT parent_id, name, t_x, t_y, t_z "
					   "FROM skeleton_joints WHERE skel_id = ? "
					   "ORDER BY offset ASC");
	q_get_joints.BindInt64(1, m_id);
	int i = 0; 
	while( q_get_joints.Step() && i < m_num_joints ) {
		m_parents[i] = q_get_joints.ColInt( 0);
		m_joint_names[i] = q_get_joints.ColText( 1 );
		m_translations[i] = q_get_joints.ColVec3( 2 );
		++i;
	} 

	ComputeTransforms();
	return true;
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

sqlite3_int64 Skeleton::ImportFromReadNode(sqlite3* db, const LBF::ReadNode& rn )
{
	if(!rn.Valid())
		return -1;

	BufferReader reader;

	skel_save_info info;
	reader = rn.GetReader();
	reader.Get(&info, sizeof(info));

	if(info.num_joints > kSaneJointCount) {
		fprintf(stderr, "Woah, crazy. Sanity check failed, a skeleton with %d joints probably isn't what you wanted (found %d).\n", kSaneJointCount, info.num_joints);
		return 0;
	}

	sql_begin_transaction(db);
	
	std::string skelname;
	LBF::ReadNode rnName = rn.GetFirstChild(LBF::SKELETON_NAME);
	if(rnName.Valid()) {
		reader = rnName.GetReader();
		int len = rnName.GetNodeDataLength() + 1;
		std::string tempStr(len+1, '\0');
		reader.Get(&tempStr[0], rnName.GetNodeDataLength());
		skelname = tempStr;
	}

	Query q_insert_skel(db, 
		  "INSERT INTO skeleton (name,root_offset_x,root_offset_y,root_offset_z,"
		  "root_rotation_a,root_rotation_b,root_rotation_c,root_rotation_r) "
		  "VALUES (:name,:x,:y,:z,:a,:b,:c,:r)");

	q_insert_skel.BindText(1, skelname.c_str());
	q_insert_skel.BindVec3(2, info.root_translation);
	q_insert_skel.BindQuaternion(5, info.root_rotation);
	q_insert_skel.Step();
	sqlite3_int64 new_id = 0;
	if(!q_insert_skel.IsError()) 
		new_id = q_insert_skel.LastRowID();
	else {
		sql_rollback_transaction(db);
		return 0;
	}

	std::string *temp_names = new std::string[info.num_joints];
	LBF::ReadNode rnJoints = rn.GetFirstChild(LBF::SKELETON_NAMES);
	if(rnJoints.Valid()) {
		readStdStringTable(rnJoints, temp_names, info.num_joints);
	}

	BufferReader translationReader;
	BufferReader parentReader;
	LBF::ReadNode rnTranslations = rn.GetFirstChild(LBF::SKELETON_TRANSLATIONS);
	if(rnTranslations.Valid()) {		
		translationReader = rnTranslations.GetReader();
	}

	LBF::ReadNode rnParents = rn.GetFirstChild(LBF::SKELETON_PARENTS);
	if(rnParents.Valid()) {
		parentReader = rnParents.GetReader();
	}

	Query q_insert_joints(db,
						  "INSERT INTO skeleton_joints (skel_id, parent_id, offset, "
						  "name, t_x, t_y, t_z, weight ) "
						  "VALUES (:skel_id, :parent_id, :offset, :name, :t_x, :t_y, :t_z, 1.0 )");
	q_insert_joints.BindInt64(1, new_id);
	for(int i = 0; i < info.num_joints; ++i)
	{
		q_insert_joints.Reset();

		int parent = -1;
		Vec3 trans(0,0,0) ;	
		translationReader.Get(&trans, sizeof(Vec3));
		parentReader.Get(&parent, sizeof(int));

		q_insert_joints.BindInt(2, parent);
		q_insert_joints.BindInt(3, i);
		q_insert_joints.BindText(4, temp_names[i].c_str());
		q_insert_joints.BindVec3(5, trans);
		q_insert_joints.Step();

		if(q_insert_joints.IsError()) {
			delete[] temp_names;
			sql_rollback_transaction(db);
			return 0;
		}
	}
	delete[] temp_names;

	sql_end_transaction(db);
	return new_id;
}

////////////////////////////////////////////////////////////////////////////////
SkeletonWeights::SkeletonWeights(sqlite3* db, sqlite3_int64 skel_id)
	: m_db(db)
	, m_skel_id(skel_id)
	, m_set_statement(db)
	, m_num_weights(0)
	, m_cached_weights(0)
{
	m_set_statement.Init("UPDATE skeleton_joints SET weight=? WHERE offset=? AND skel_id = ?");
	m_set_statement.BindInt64(3, skel_id);

	Query query(db, "SELECT count(*) FROM skeleton_joints WHERE skel_id=?");
	query.BindInt64(1, m_skel_id);
	if(query.Step())
	{
		m_num_weights = query.ColInt(0) ;
		m_cached_weights = new float[m_num_weights];
		for(int i = 0; i < m_num_weights; ++i) m_cached_weights[i] = 1.f;
	}

	if(m_num_weights > 0) 
	{
		Query get_weights(db,"SELECT offset,weight FROM skeleton_joints WHERE skel_id = ? ORDER BY offset ASC");
		get_weights.BindInt64(1, m_skel_id);
		while( get_weights.Step() )
		{
			int offset = get_weights.ColInt(0);
			ASSERT(offset < m_num_weights);
			m_cached_weights[offset] = get_weights.ColDouble(1);
		}
	}
}

SkeletonWeights::~SkeletonWeights()
{
	delete[] m_cached_weights;
}

void SkeletonWeights::SetJointWeight(int idx, float weight) 
{
	sql_begin_transaction(m_db);

	m_set_statement.Reset();
	m_set_statement.BindInt(2, idx);
	m_set_statement.BindDouble(1, weight);
	m_set_statement.Step();
	
	if(!m_set_statement.IsError()) {
		m_cached_weights[idx] = weight;
	}

	sql_end_transaction(m_db);
}

float SkeletonWeights::GetJointWeight(int idx) const 
{
	return m_cached_weights[idx];
}

LBF::WriteNode* SkeletonWeights::CreateWriteNode() const 
{	
	const int num_joints = m_num_weights;

	LBF::WriteNode* weightsNode = new LBF::WriteNode(LBF::SKELETON_JOINT_WEIGHTS, 0, num_joints*sizeof(float));
	BufferWriter writer = weightsNode->GetWriter();

	for(int i = 0; i < num_joints; ++i)
	{
		writer.Put(&m_cached_weights[i], sizeof(float));
	}
	return weightsNode;
}

void SkeletonWeights::ImportFromReadNode( const LBF::ReadNode& rn )
{
	if(!rn.Valid()) return ;
	BufferReader weightReader = rn.GetReader();

	int num_joints = rn.GetNodeDataLength() / sizeof(float);

	float* cached_weights = new float[num_joints];
	for(int i = 0; i < num_joints; ++i) cached_weights[i] = 1.f;

	Query update(m_db, "UPDATE skeleton_joints SET weight = ? WHERE skel_id = ? and offset = ?");
	update.BindInt64(2, m_skel_id);

	sql_begin_transaction(m_db);
	for(int i = 0; i < num_joints; ++i) {
		update.Reset();

		float w = 1.f;
		weightReader.Get(&w, sizeof(w));

		update.BindDouble(1, w);
		update.BindInt(3, i);
		update.Step();

		cached_weights[i] = w;
	}
	sql_end_transaction(m_db);

	// replace the cache with the imported data
	m_num_weights = num_joints;
	delete[] m_cached_weights; m_cached_weights = cached_weights;
}




