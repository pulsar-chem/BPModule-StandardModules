#ifndef _GUARD_INTEGRAL_COMMON_HPP_
#define _GUARD_INTEGRAL_COMMON_HPP_

#include <pulsar/system/BasisSet.hpp>
#include <pulsar/datastore/CacheData.hpp>


std::shared_ptr<pulsar::system::BasisSet>
NormalizeBasis(pulsar::datastore::CacheData & cache,
               pulsar::output::OutputStream & out,
               const pulsar::system::BasisSet & bs);



#endif
