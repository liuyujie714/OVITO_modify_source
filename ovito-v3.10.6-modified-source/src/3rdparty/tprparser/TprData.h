#ifndef TPRDATA_H
#define TPRDATA_H

#include <vector>
#include <string>
#include <array>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <tuple> // std::tie

#include "define.h"
#include "Enum.h"

using vecI2D = std::vector<std::vector<int>>;
using vecF2D = std::vector<std::vector<float>>;
using vecU2D = std::vector<std::vector<unsigned short>>;


//< code can reach
static inline void unreachable() {
#if HAS_BUILTIN_UNREACHABLE
	__builtin_unreachable();
#elif defined(_MSC_VER)
	__assume(false);
#else
	throw std::runtime_error("entered unreachable code");
#endif
}

// bonded pairs and ff parameters, such as bonds, angle, dihedrals
struct Bonded
{
	// bond
	Bonded(int ta, int tb, int functype, const std::vector<float> &ffparam)
		: a(ta), b(tb), ifunc(functype), ff { ffparam }
	{
		if (a > b) std::swap(a, b);
	}

	// angle
	Bonded(int ta, int tb, int tc, int functype, const std::vector<float>& ffparam)
		: a(ta), b(tb), c(tc), ifunc(functype), ff{ ffparam }
	{
		if (a > c) std::swap(a, c);
	}

	// dihedral
	Bonded(int ta, int tb, int tc, int td, int functype, const std::vector<float>& ffparam)
		: a(ta), b(tb), c(tc), d(td), ifunc(functype), ff{ ffparam }
	{
		// small x x big
		if (a > d)
		{
			std::swap(a, d);
			std::swap(b, c);
		}
	}

	// return a, b, c, d according to index 0-3
	int &operator[](size_t idx)
	{
		assert(idx >= 0 && idx < 4);
		switch (idx)
		{
		case 0: return a;
		case 1: return b;
		case 2: return c;
		case 3: return d;
		}
		unreachable();
	}
	// const version
	int operator[](size_t idx) const
	{
		assert(idx >= 0 && idx < 4);
		switch (idx)
		{
		case 0: return a;
		case 1: return b;
		case 2: return c;
		case 3: return d;
		}
		unreachable();
	}

	// 不去重复用vector
	// a b c d
	//bool operator<(const Bonded& rhs) const
	//{
	//	return std::tie(a, d, b, c) < std::tie(rhs.a, rhs.d, rhs.b, rhs.c);
	//}

	int					a = 0; // atom1
	int					b = 0; // atom2
	int					c = 0; // atom3
	int					d = 0; // atom4
	int					ifunc = 0; // the function type id, 1,2,,,
	std::vector<float>	ff {}; // ff parameters
};

//< Non-Bonded pairs
struct NonBonded
{
	NonBonded() = default; // used for std::vector .resize()

	// pairs set
	NonBonded(int i, int j, int functype, const std::vector<float>& ffparam)
		: a(i), b(j), ifunc(functype), ff{ ffparam } {}

	// LJ set
	NonBonded(int functype, const std::vector<float>& ffparam)
		: ifunc(functype), ff{ ffparam } {}
	// LJ set by copy constructor
	NonBonded(const NonBonded& rhs) noexcept
	{
		a = rhs.a; b = rhs.b; ifunc = rhs.ifunc;
		ff = rhs.ff;
	}

	int &operator[](size_t idx)
	{
		assert(idx >= 0 && idx < 2);
		return idx == 0 ? a : b;
	}

	// const version
	int operator[](size_t idx) const 
	{
		assert(idx >= 0 && idx < 2);
		return idx == 0 ? a : b;
	}

	int					a = 0; // atom1 of [ pairs ]
	int					b = 0; // atom2 of [ pairs ]
	int					ifunc = 0;
	std::vector<float>	ff{}; // non-bonded parameters
};


struct TprData
{
	// POD clear zero
	TprData() = default;

	~TprData() {}

	int					prec; //< the precision of tpr, 4 or 8
	int					filever; //< the version of file format, fver
	int					vergen; //< the verions of generation code, fgen
	int					natoms; //< the total natoms
	int					ngtc; //< The number of temperature coupling groups.
	int					fep_state; //< fep state
	float				lambda; //< lambda
	bool				bIr; //< if has ir 
	bool				bTop; //< if has top 
	bool				bX; //< if has coordinates 
	bool				bV; //< if has velocity
	bool				bF; //< if has force
	bool				bBox; //< if has box 
	bool				bInter; //< if has inter-molecular bonds
	std::vector<float>	box = {}; //< box size
	std::vector<char>   symtab;//< symb name, truncate to SAVELEN characters
	int					nmoltypes, nmolblock;
	int					atnr; // the number of LJ type 

	std::vector<int>	atomsinmol;
	std::vector<int>	resinmol;
	std::vector<int>	molnames;
	std::vector<int>	molbtype;
	std::vector<int>	molbnmol;
	std::vector<int>	molbnatoms; // 每个单分子有多少个原子构成
	vecF2D				charges;
	vecF2D				masses;
	vecI2D				resids;
	std::vector<int>	trueresids; // actually residues number in tpr
	vecI2D				ptypes;
	vecU2D				types; // LJ param type id
	vecI2D				atomnameids;
	vecI2D				atomtypeids;
	vecI2D				resnames;
	vecI2D				atomicnumbers;

	// mdp parameters
	struct
	{
		PbcType				pbc = PbcType::Unset; //< which pbc type
		bool				pbcmol; //< periodic-molecules
		int64_t				nsteps; // the number of simulation steps 
		int64_t				init_step; // simulation init steps
		int					simulation_part;
		int					nstcalcenergy;
		int					cutoff_scheme; // int to enum
		int					nstlist;
		int					nstcomm;
		int					comm_mode; // int to enum, 0=Linear, 1=Angular
		int					nstcgsteep; // Number of steps after which a steepest descents step is done while doing cg
		int					nbfgscorr; // Number of corrections to the Hessian to keep
		int					nstlog; // number of log steps
		int					nstxout; // number of trr coordinates steps
		int					nstvout; // number of velocity steps
		int					nstfout; // number of force steps
		int					nstenergy; // number of energy output steps
		int					nstxout_compressed; // number of xtc coordinates steps
		double				init_t = 0.0; // init time, ps
		double				dt = 0.0; // time steps, ps

		float				x_compression_precision; /// precision of xtc coordinates
		float				verletbuf_tol; // tolerance of verlet buffer
		float				verletBufferPressureTolerance;
		float				rlist;
		int					coulombtype; // int to enum, 0=Cut, 1=RF, 3=Pme
		int					coulomb_modifier; // int to enum, 0=PotShiftVerletUnsupported, 1=PotShift, 2=None
		float				rcoulomb_switch;
		float				rcoulomb;
		int					vdwtype;// int to enum, 0=Cut, 1=Switch,2=Shift, ...
		int					vdw_modifier; // int to enum


		float				rvdw_switch;
		float				rvdw;
		int					eDispCorr;
		float				epsilon_r;
		float				epsilon_rf;
		float				tabext;

		bool				implicit_solvent = false; // if has implicit solvent

		float				fourier_spacing;
		int					nkx;
		int					nky;
		int					nkz;
		int					pme_order;
		float				ewald_rtol;
		float				ewald_rtol_lj;
		int					ewald_geometry; // int to enum, 0=3D, 1=3DC
		float				epsilon_surface;
		int					ljpme_combination_rule; // int to enum, 0=Geom, 1=LB
		bool				bContinuation;
		// int to enum, 0=No,1=Berendsen,2=NoseHoover,3=Yes,4=Andersen,5=AndersenMassive
		// 6=VRescale
		int					etc;

		int					nsttcouple;
		int					nstpcouple;
		int					epc; // Pressure coupling algorithm，int to enum, 0=No, 1=Berendsen, 2=ParrinelloRahman,5=CRescale
		int					epct; //Pressure coupling type, int to enum, 0=Isotropic, 1=SemiIsotropic
		float				tau_p;
		float				ref_p[DIM * DIM] = { 0 };
		float				compress[DIM * DIM] = { 0 };
		float				posres_com[DIM] = { 0 };
		float				posres_comB[DIM] = { 0 };
		int					refcoord_scaling; // int to enum,0=No,1=All,2=Com

		float				shake_tol; // tolerance of shake
		int					efep; // int to enum
		int					n_lambda = 0; //The number of foreign lambda points
		bool				bSimTemp;// if has simulation temperature
		int					eSimTempScale; // enum to int
		float				simtemp_high;
		float				simtemp_low;

		bool				bExpanded = false; // Whether expanded ensembles are used

		// em
		float				em_stepsize;
		float				em_tol;
		int64_t				ld_seed = 0;

		// deform
		float				deform[DIM * DIM] = { 0 };
		float				cos_accel = 0;
		int					userint1, userint2, userint3, userint4;
		float				userreal1, userreal2, userreal3, userreal4;


		int					ngacc = 0; // 加速组个数
		int					ngfrz = 0; // 冻结组个数
		int					ngener = 0; // 能量组个数

		// 控温部分，维度ngtc
		int					ngtc = 0; // 控温组数目
		int					nhchainlength = 1;
		std::vector<float>  nrdf; // 每个组自由度
		std::vector<float>  ref_t; // 每个组参考温度
		std::vector<int>    annealing; // 每个组模拟退火类型, enum to int
		std::vector<int>    anneal_npoints; // 每个组模拟退火点数
		vecF2D				anneal_time; // 每个组模拟退火时间点
		vecF2D				anneal_temp; // 每个组模拟退火温度点
		std::vector<float>	tau_t;  // 每个组模拟退火时间常数

		std::vector<std::array<int, DIM>> nFreeze; // 每个组在三个方向时候被冻结，维度ngfrz
		std::vector<std::array<float, DIM>> acceleration; // 每个组在三个方向时候被冻结，维度ngacc
		std::vector<int>	egp_flags; // 能量组每对之间的Exclusions/tables，维度ngener*ngener

		// 支持新和旧版本gmx电场参数保存. DIM*4 每个维度四个数：E0, omega, t0, sigma
		//! 即使mdp中没设置电场，tpr中依然有电场部分可读，但值都是0
		std::vector<float>	elec_field;
		struct
		{
			int n = 0, nt = 0; // 时间，空间项数，<=1
		} elec_old_gmx[DIM]; //低版本tpr用
		//! 高版本tpr用：
		int					elec_nf = 0; // 电场项，必须是1
		int					elec_ne = 0; // applied-forces项目，必须是>=1
	} ir;


	struct
	{
		vecI2D			    interactionlist[F_NRE];
		std::vector<int>	nr[F_NRE];
	}
	ilist,					// 分子相互作用列表
	inter_molecular_ilist;	// 全局指定的分子间相互作用

	// 原子属性
	struct
	{
		std::vector<float>			x; //< coordinates
		std::vector<float>			v; //< velocity
		std::vector<float>			f; //< force
		std::vector<std::string>	atomname;
		std::vector<std::string>	resname;
		std::vector<std::string>	atomtypename; // atom type name from .ff
		std::vector<int>			atomtypenumber; // atomtype number, -1=unknown, 0=VSite
		std::vector<int>			resid;
		std::vector<float>			mass;
		std::vector<float>			charge;
		std::vector<int>			atomnumber; // atomic number, -1=unknown, 0=VSite
		std::vector<unsigned short>	type; // unused ?
	} atoms;

	//! 此处没有进行去重复，一个角可以存在多类参数
	// bonds (1-based)
	std::vector<Bonded>		bonds;
	// angles (1-based)
	std::vector<Bonded>		angles;
	// proper dihedrals (1-based)
	std::vector<Bonded>		dihedrals;
	// improper dihedrals (1-based)
	std::vector<Bonded>		impropers;
	// pairs/LJ_14
	std::vector<NonBonded>	pairs;
	// only the atomtype LJ parameters
	std::vector<NonBonded>	atomtypesLJ; 
	// all atoms LJ parameters
	std::vector<NonBonded>	ljparams; 

	// mdp属性位置, 所有变量都必须初始化为0
	struct
	{
		long		nsteps = 0; //< the started nsteps position in tpr
		long		dt = 0; //< the started dt position in tpr
		long		x = 0; //< the started atom coordinates position in tpr
		long		v = 0; //< the started atom velocity position in tpr
		long		f = 0; //< the started atom force position in tpr
		long		box = 0; //< the box position in tpr
		long		ef = 0; //< the electric field started position in tpr


		// 压力设置参数位置
		struct
		{
			long	box_rel = 0; //< the started vector position for preserve box shape, is DIM*DIM vector
			long	epc = 0; //< the started pressure coupling method position
			long	epct = 0; //< the started pressure coupling type position
			long	tau_p = 0; //< the started tau_p position
			long	ref_p = 0; //< the started ref pressure value position, is DIM*DIM vector
			long	compress = 0; //< the started compressibility value position, is DIM*DIM vector
			long	deform = 0;//< the started deform value positon, is DIM*DIM vector

			//< return False if get all parameters
			bool empty() const
			{
				return !(box_rel && epc && epct && tau_p && ref_p && compress && deform);
			}
		} press;
		
		// 温度设置参数位置
		struct 
		{
			long	g_ngtc = 0; //< the started number of temperature coupling group position in tpr header, g_ngtc==ir->ngtc
			long	etc = 0; //< the started temperature coupling type position, enum to int, 0=No, 1=Berendsen,2=NoseHoover,6=VRescale
			long	ngtc = 0; //< the started number of temperature coupling group position
			long    nhchainlength = 0; //< the Nose-Hoover chain length if use Nose-Hoover temperature coupling
			long	ref_t = 0; //< the started ref temperature position, is ir->ngtc vector
			long	tau_t = 0; //< the started temperature coupling constant position, is ir->ngtc vector

			//< return True if can not read temperature position due to pull code, AWH have not yet finish in tpr reader (TODO)
			bool empty() const
			{
				return !(ref_t && tau_t && etc && ngtc && g_ngtc);
			}
		} temperature;
		
		// 单个整数属性mdp设置位置
		struct
		{
			//! 定义变量顺序和类型必须和枚举顺序完全一致
			long			nstlog = 0; // started 
			long			nstxout = 0;
			long			nstvout = 0;
			long			nstfout = 0;
			long			nstenergy = 0;
			long			nstxout_compressed = 0;

			// must be data_->filever >= 71
			long			nsttcouple = 0;
			long			nstpcouple = 0;

			//must be data_->filever >= 67
			long			nstcalcenergy = 0;

			//< return True if can not read any one position
			bool empty() const
			{
				return !(nstlog && nstxout && nstvout && nstfout && nstenergy && nstxout_compressed && nsttcouple && nstpcouple && nstcalcenergy);
			}
		} integer;

	} property;
};


#endif // !TPRDATA_H
