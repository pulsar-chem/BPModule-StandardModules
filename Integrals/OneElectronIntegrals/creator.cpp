#include "Overlap.hpp"
#include "Dipole.hpp"
#include "KineticEnergy.hpp"
#include "CoreBuild.hpp"
#include "OneElectronPotential.hpp"
#include "OneElectronProperty.hpp"

using pulsar::modulemanager::ModuleCreationFuncs;


extern "C" {

ModuleCreationFuncs InsertSupermodule(void)
{
    ModuleCreationFuncs cf;
    cf.AddCppCreator<Overlap>("Overlap");
    cf.AddCppCreator<Dipole>("Dipole");
    cf.AddCppCreator<KineticEnergy>("KineticEnergy");
    cf.AddCppCreator<OneElectronPotential>("OneElectronPotential");
    cf.AddCppCreator<CoreBuild>("CoreBuild");
    cf.AddCppCreator<OneElectronProperty>("OneElectronProperty");
    return cf;
}



}

