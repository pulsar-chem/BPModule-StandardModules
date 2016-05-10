#include <pulsar/output/OutputStream.hpp>
#include <pulsar/system/BasisSet.hpp>
#include <pulsar/system/AOIterator.hpp>
#include <pulsar/math/Cast.hpp>
#include <eigen3/Eigen/Dense>
#include "Methods/SCF/CoreGuess.hpp"
#include "Methods/SCF/SCF_Common.hpp"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::SelfAdjointEigenSolver;

using namespace pulsar::modulemanager;
using namespace pulsar::modulebase;
using namespace pulsar::exception;
using namespace pulsar::system;
using namespace pulsar::math;
using namespace pulsar::output;
using namespace pulsar::datastore;


namespace pulsarmethods{


std::vector<double> CoreGuess::Deriv_(size_t order)
{
    if(order != 0)
        throw NotYetImplementedException("CoreGuess with deriv != 0");

    // make sure stuff is set in wavefunction
    const Wavefunction & iwfn = InitialWfn();

    if(!iwfn.GetSystem())
        throw GeneralException("System is not set!");


    // get the basis set
    const System & sys = *(iwfn.GetSystem());
    std::string bstag = Options().Get<std::string>("BASIS_SET");

    out.Output("Obtaining basis set %? from system\n", bstag);
    const BasisSet bs = sys.GetBasisSet(bstag);
    size_t nao = bs.NFunctions();
    size_t nshell = bs.NShell();
    size_t maxnfunc = bs.MaxNFunctions();
    size_t maxnfunc2 = maxnfunc * maxnfunc;

    out.Output("NAO: %? nshell: %?\n", nao, nshell);
    bs.Print(out);
    

    ///////////////////////////////////////////
    // Load the one electron integral matrices
    // (and nuclear repulsion)
    ///////////////////////////////////////////
    // Nuclear repulsion
    auto mod_nuc_rep = CreateChildFromOption<SystemIntegral>("KEY_NUC_REPULSION");
    double nucrep;
    size_t n = mod_nuc_rep->Calculate(0, &nucrep, 1);

    /////////////////////// 
    // Overlap
    auto mod_ao_overlap = CreateChildFromOption<OneElectronIntegral>("KEY_AO_OVERLAP");
    mod_ao_overlap->SetBases(bstag, bstag);
    MatrixXd overlap_mat = FillOneElectronMatrix(mod_ao_overlap, bs);

    // diagonalize the overlap
    SelfAdjointEigenSolver<MatrixXd> esolve(overlap_mat);
    MatrixXd s_evec = esolve.eigenvectors();
    VectorXd s_eval = esolve.eigenvalues();

    // not sure an easier way to do this
    for(size_t i = 0; i < s_eval.size(); i++)
        s_eval(i) = 1.0/sqrt(s_eval(i));

    // the S^(-1/2) matrix
    MatrixXd S12 = s_evec * s_eval.asDiagonal() * s_evec.transpose();

    //////////////////////////// 
    // One-electron hamiltonian
    auto mod_ao_core = CreateChildFromOption<OneElectronIntegral>("KEY_AO_COREBUILD");
    mod_ao_core->SetBases(bstag, bstag);
    MatrixXd Hcore = FillOneElectronMatrix(mod_ao_core, bs);


    //////////////////////////
    // Occupations
    //////////////////////////
    double nelec_d = sys.GetNElectrons();
    if(!IsInteger(nelec_d))
        throw GeneralException("Can't handle non-integer occupations", "nelectrons", nelec_d);
    size_t nelec = numeric_cast<size_t>(nelec_d);

    out.Output("Number of electrons: %?\n", nelec);


    // Block some eigen matrices, etc, by irrep and spin
    BlockByIrrepSpin<MatrixXd> cmat, dmat;
    BlockByIrrepSpin<VectorXd> epsilon;
    BlockByIrrepSpin<VectorXd> occ;

    // Fill in the occupations
    occ = FindOccupations(nelec);


    // 2. Initial fock matrix
    MatrixXd F0 = S12.transpose() * Hcore * S12.transpose();
    SelfAdjointEigenSolver<MatrixXd> fsolve(F0);
    MatrixXd C0 = fsolve.eigenvectors();
    VectorXd e0 = fsolve.eigenvalues();

    // Tranform C0
    C0 = S12*C0;


    // The initial C matrix is the same for all spins
    // Use the irrep/spin from occupations
    for(auto s : occ.GetSpins(Irrep::A))
        cmat.Set(Irrep::A, s, C0);


    // Calculate the initial Density
    for(auto s : cmat.GetSpins(Irrep::A))
    {
        const auto & c = cmat.Get(Irrep::A, s);
        const auto & o = occ.Get(Irrep::A, s);

        MatrixXd d(c.rows(), c.cols());

        for(size_t i = 0; i < c.rows(); i++)
        for(size_t j = 0; j < c.cols(); j++)
        {
            d(i,j) = 0.0;
            for(size_t m = 0; m < o.size(); m++)
                d(i,j) += o(m) * c(i,m) * c(j,m);
        }

        dmat.Take(Irrep::A, s, std::move(d));
    }

    // initial energy
    double energy = nucrep;
    for(auto s : dmat.GetSpins(Irrep::A))
    {
        const auto & d = dmat.Get(Irrep::A, s);
        for(size_t i = 0; i < d.rows(); i++)
        for(size_t j = 0; j < d.cols(); j++)
            energy += d(i,j) * Hcore(i,j);
    }


    out.Output("Initial guess energy: %12.8e\n", energy);


    // save the occupations and other initial guess info
    IrrepSpinVectorD final_occ = occ.TransformType<SimpleVectorD>(EigenToSimpleVector);
    IrrepSpinVectorD final_epsilon = epsilon.TransformType<SimpleVectorD>(EigenToSimpleVector);
    IrrepSpinMatrixD final_cmat = cmat.TransformType<SimpleMatrixD>(EigenToSimpleMatrix);

    auto & fwfn = FinalWfn();
    fwfn.SetCMat(std::move(final_cmat));
    fwfn.SetOccupations(std::move(final_occ));
    fwfn.SetEpsilon(std::move(final_epsilon));

    return {energy};
}
    

}//End namespace
