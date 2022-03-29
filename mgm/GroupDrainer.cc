#include "mgm/GroupDrainer.hh"
#include "mgm/groupbalancer/StdDrainerEngine.hh"

namespace eos::mgm {

GroupDrainer::GroupDrainer(std::string_view spacename) : mSpaceName(spacename),
                                                         mEngine(std::make_unique<group_balancer::StdDrainerEngine>())

{}

void
GroupDrainer::GroupDrain()
{}

} // eos::mgm
