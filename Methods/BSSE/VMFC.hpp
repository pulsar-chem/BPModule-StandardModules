#ifndef VMFC_HPP_
#define VMFC_HPP_

#include<vector>
#include<bpmodule/modulebase/EnergyMethod.hpp>

namespace bpmethods{
/** \brief This class computes the derivatives of the Valiron-Mayer functional counterpoise procedure
 *
 *  VMFC is arguably the correct way to counterpoise correct a system; however, 
 *  it is very expensive.  VMFC(n), where (n) means the expansion has been
 *  truncated at order (n) can be written
 *  as the sum of all BSSE-free \f$i\f$-body BSSE free interactions, 
 *  \f$\Delta E^{(i)}\f$, up to \f$i=n\f$:
 *  \f[
 *    E=\sum_{i=1}^n\Delta E^{(i)},
 *  \f]
 *  Each \f$\Delta E^{(i)}\f$ is written almost in the usual MBE way, so trimers
 *  for example are:
 *  \f[
 *    \Delta E^{(i)}=\sum_{I<J<K}E_{IJK}-\Delta E_{IJ}^{IJK}-
 *                   \sum_{IJ\in \lbrace IJK\rbrace}\Delta E_{IJ}^{IJK}-
                     \sum_{I \in \lbrace IJK\rbrace}-E_I^{IJK}
 *  \f]
 *   except that the each computation required for the given \f$i\f$-body 
 *   interaction is performed in the
 *   \f$i\f$-body basis set.
 *   For a system comprised of \f$N\f$ monomers, this means we have to perform:
 *   \f[
 *      \sum_{i=1}^n{N\choose i}\left(2^i-1\right)
 *   \f]
 *   computations (there are \f$N\f$ choose \f$i\f$, \f$i\f$-body interactions, 
 *   each of which involves \f$2^i-1\f$ computations).
 * 
 *   Implementation wise, the bulk of the heavy-lifting is done by MIM, for 
 *   which arbitrary order derivatives are available, hence
 *   we have arbitrary order derivatives as well (individual computations will 
 *   be done by finite-diffence if their derivatives are not available).
 * 
 *   For MIM we need the coefficients.  Each purely real system appears with
 *   a coefficient of 1.  For the remainder of the systems, we can use a
 *   result from Richard, Lao, Herbert. JCP 141 (2014) 014108 that tells us
 *   that for a given n-body interaction, a (n-m)-body interaction will have
 *   a coefficient of \f$(-1)^m\f$.  In otherwords, if the m is the opposite
 *   parity of n the coefficient is -1. This is only true if we are considering
 *   all sub-interactions within the n-mer.
 * 
 */
class VMFC : public bpmodule::modulebase::EnergyMethod {
   private:
      typedef bpmodule::modulebase::EnergyMethod Base_t;
   public:
      using Base_t::EnergyMethod;
      virtual std::vector<double> Deriv_(size_t Order);
};

}//End namespace

#endif /*End header guard VMFC_HPP_*/
