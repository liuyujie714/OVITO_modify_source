#ifndef UTILS_H
#define UTILS_H

#include <cstdio>
#include <vector>
#include <utility> // std::pair

#include "define.h"

//! \brief return bond function type id and force parameters. 
//! includes constraint derived from bonds
//! \return return std::pair(-1, {}) if failed
std::pair<int, std::vector<float>> get_bond_type(int ftype, const t_iparams* param);

//! \brief return angle function type id and force parameters,
//! \return return std::pair(-1, {}) if failed
std::pair<int, std::vector<float>> get_angle_type(int ftype, const t_iparams* param);

//! \brief return dihedral function type id and force parameters,
//! \return return std::pair(-1, {}) if failed
std::pair<int, std::vector<float>> get_dihedral_type(int ftype, const t_iparams* param);

//! \brief return impropers dihedral function type id and force parameters,
//! \return return std::pair(-1, {}) if failed
std::pair<int, std::vector<float>> get_improper_type(int ftype, const t_iparams* param);

//! \brief return nonbonded (LJ/LJ-14) function type id and force parameters,
//! ifunc=1 is LJ-14, ifunc=3 is LJ
//! \return return std::pair(-1, {}) if failed
std::pair<int, std::vector<float>> get_nonbonded_type(int ftype, const t_iparams* param);

//! \brief safely fopen 
FILE* efopen(const char* fname, const char* mod);

#endif // !UTILS_H
