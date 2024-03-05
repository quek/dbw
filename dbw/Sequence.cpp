#include "Sequence.h"
#include <map>

int Sequence::_no = 0;
std::map<NekoId, std::weak_ptr<Sequence>> Sequence::nekoIdSequenceMap;

