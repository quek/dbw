#include "AutomationPoint.h"

AutomationPoint::AutomationPoint(const nlohmann::json& json) : SequenceItem(json) {
}

void AutomationPoint::prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) {
    (void*)processBuffer;
    printf("%f", begin + end + clipTime + clipDuration + oneBeatSec);

}
