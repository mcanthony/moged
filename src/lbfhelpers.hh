#ifndef INCLUDED_lbf_helpers_HH
#define INCLUDED_lbf_helpers_HH

// Stores length and offset for each string to make loading easy.
LBF::WriteNode* createStdStringTableNode( int type, int id, const std::string* array, int len);
void readStdStringTable( const LBF::ReadNode& rn, std::string* table, int len);

#endif
