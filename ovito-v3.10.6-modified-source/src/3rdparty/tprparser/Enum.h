#ifndef ENUM_H
#define ENUM_H

#if defined(_MSC_VER) || defined(_WIN32)
#include <string.h>
#define mystricmp _stricmp
#else
#include <strings.h>
#define mystricmp strcasecmp
#endif

enum class PbcType : int
{
	Xyz = 0, //!< Periodic boundaries in all dimensions.
	No = 1, //!< No periodic boundaries.
	XY = 2, //!< Only two dimensions are periodic.
	Screw = 3, //!< Screw.
	Unset = 4, //!< The type of PBC is not set or invalid.
	Count = 5,
	Default = Xyz
};

//< pressure coupling methods
enum class PressureCoupling : int
{
	No,
	Berendsen,
	ParrinelloRahman,
	Isotropic,
	Mttk,
	CRescale,
	Count
};
static const char* c_PressureCoupling[static_cast<int>(PressureCoupling::Count)] =
{
	"No", "Berendsen", "ParrinelloRahman", "Isotropic", "Mttk", "CRescale"
};

//< pressure coupling type
enum class PressureCouplingType : int
{
	Isotropic,
	SemiIsotropic,
	Anisotropic,
	Count
};
static const char* c_PressureCouplingType[static_cast<int>(PressureCouplingType::Count)] =
{
	"Isotropic", "SemiIsotropic", "Anisotropic"
};

//< temperature coupling methods
enum class TemperatureCoupling : int
{
	No,
	Berendsen,
	NoseHoover,
	Yes,
	Andersen,
	AndersenMassive,
	VRescale,
	Count,
};
static const char* c_TemperatureCoupling[static_cast<int>(TemperatureCoupling::Count)] =
{
	"No", "Berendsen", "NoseHoover", "Yes", "Andersen", "AndersenMassive", "VRescale"
};


// Integer mdp
enum class ParamsInteger : int
{
	nstlog, nstxout, nstvout, nstfout,
	nstenergy, nstxout_compressed,
	nsttcouple, nstpcouple, nstcalcenergy,
	Count,
};
static const char* c_mdp_integer[static_cast<int>(ParamsInteger::Count)] =
{
	"nstlog", "nstxout", "nstvout", "nstfout",
	"nstenergy", "nstxout_compressed" ,
	"nsttcouple", "nstpcouple", "nstcalcenergy"
};

// vector of tpr, X or V or F
enum class VecProps : int
{
	x,
	v, 
	f,
	m, // the mass of atoms
	q, // the charge of atoms
	box, // the box vector
	ef, // electric field
	Count
};
static const char* c_mdp_vector[static_cast<int>(VecProps::Count)] =
{
	"x", "v", "f", "m", "q", "box", "ef"
};

// int vector of tpr, such resid
enum class IVectorProps : int
{
	resid,
	atnum, // atomtype number
	atomicnum, // atomic number
	Count
};
static const char* c_int_vector[static_cast<int>(IVectorProps::Count)] =
{
	"resid", "atnum", "atomicnum"
};

// vector of tpr, resname / atomname / atomtype name
enum class StringType : int
{
	res,
	atom,
	type, // atomtype name
	Count
};
static const char* c_name_vector[static_cast<int>(StringType::Count)] =
{
	"res", "atom", "type"
};


// type of bonded
enum class BondedType : int
{
	bonds,
	angles,
	dihedrals,
	impropers,
	Count
};
static const char* c_bonded_type[static_cast<int>(BondedType::Count)] =
{
	"bonds", "angles", "dihedrals", "impropers"
};

//< Non bonded type
enum class NonBondedType : int
{
	LJ, 
	atomtype,	// only [ atomtypes ]  
	LJ_14,		// that is [ pairs ]
	Count
};
static const char* c_nonbonded_type[static_cast<int>(NonBondedType::Count)] =
{
	"lj", "type", "pairs"
};


//< check key words in a c_string ignore case, return enum value if find, else return ENUM::Count
template<typename ENUM, const int count = static_cast<int>(ENUM::Count)>
static inline ENUM check_string(const char* str, const char* arr[])
{
	for (int i = 0; i < count; i++)
	{
		if (!mystricmp(str, arr[i])) return static_cast<ENUM>(i);
	}
	return ENUM::Count;
}



#endif // !ENUM_H
