#include "Methods/SCF/HFIterate.hpp"
#include "pulsar/modulebase/All.hpp"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::SelfAdjointEigenSolver;

using namespace pulsar::datastore;
using namespace pulsar::system;
using namespace pulsar::math;
using namespace pulsar::exception;
using namespace pulsar::output;
using namespace pulsar::modulebase;


namespace pulsarmethods {


void HFIterate::Initialize_(const Wavefunction & wfn)
{
    if(!wfn.system)
        throw GeneralException("System is not set!");

    // get the basis set
    const System & sys = *(wfn.system);
    std::string bstag = Options().Get<std::string>("BASIS_SET");
    const BasisSet bs = sys.GetBasisSet(bstag);

    /////////////////////////////////////
    // The one-electron integral cacher
    /////////////////////////////////////
    auto mod_ao_cache = CreateChildFromOption<OneElectronMatrix>("KEY_ONEEL_MAT");

    /////////////////////// 
    // Overlap
    /////////////////////// 
    const std::string ao_overlap_key = Options().Get<std::string>("KEY_AO_OVERLAP");
    auto overlapimpl = mod_ao_cache->Calculate(ao_overlap_key, 0, wfn, bs, bs);
    std::shared_ptr<const MatrixXd> overlap_mat = convert_to_eigen(overlapimpl.at(0));  // .at(0) = first (and only) component
    S12_ = FormS12(*overlap_mat);

    initialized_ = true;
}


Wavefunction HFIterate::Next_(const Wavefunction & wfn, const IrrepSpinMatrixD & fmat)
{
    if(!initialized_)
        Initialize_(wfn);

    // The density and C matrix we are returning
    IrrepSpinMatrixD Cmat;
    IrrepSpinVectorD epsilon;

    // Diagonalize, etc
    for(auto ir : fmat.GetIrreps())
    for(auto s : fmat.GetSpins(ir))
    {
        std::shared_ptr<const MatrixXd> fptr = convert_to_eigen(fmat.Get(ir, s));
        const MatrixXd & f = *fptr;

        MatrixXd Fprime = S12_.transpose() * f * S12_;

        SelfAdjointEigenSolver<MatrixXd> fsolve(Fprime);
        MatrixXd c = fsolve.eigenvectors();
        VectorXd e = fsolve.eigenvalues();
        c = S12_*c;

        Cmat.Take(ir, s, std::make_shared<EigenMatrixImpl>(std::move(c)));
        epsilon.Take(ir, s, std::make_shared<EigenVectorImpl>(std::move(e)));
    }

    // build the density
    IrrepSpinMatrixD Dmat = FormDensity(Cmat, *wfn.occupations);

    // build the new wavefunction
    Wavefunction newwfn;
    newwfn.system = wfn.system;
    newwfn.cmat = std::make_shared<const IrrepSpinMatrixD>(std::move(Cmat));
    newwfn.opdm = std::make_shared<const IrrepSpinMatrixD>(std::move(Dmat));
    newwfn.occupations = wfn.occupations; // didn't change
    newwfn.epsilon = std::make_shared<const IrrepSpinVectorD>(std::move(epsilon));

    return newwfn;

}


} // close namespace pulsarmethods
