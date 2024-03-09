#include "AutomationPoint.h"

AutomationPoint::AutomationPoint(double value, double time) : SequenceItem(time), _value(value) {
}

AutomationPoint::AutomationPoint(const nlohmann::json& json) : SequenceItem(json) {
}

void AutomationPoint::prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) {
}
