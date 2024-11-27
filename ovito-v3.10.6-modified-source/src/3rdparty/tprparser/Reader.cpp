#include <set>
#include <cstring> // memset
#include <cmath> // pow
#include <algorithm>

#include "Reader.h"
#include "Utils.h"

bool TprReader::tpr_header()
{
	// read the first int at the first of tpr
	int tempint;
	if (!tpr_.do_int(&tempint)) return TPR_FAILED;

	// read string contains gmx version
	char filever[MAX_LEN];
	if (!tpr_.xdr_string(filever, MAX_LEN)) return TPR_FAILED;
	msg("gmx version: %s\n", filever);

	// read precision int
	if (!tpr_.do_int(&data_->prec)) return TPR_FAILED;
	msg("gmx precision: %s\n", data_->prec == sizeof(float) ? "float" : "double");
	if (data_->prec != sizeof(float) && data_->prec != sizeof(double))
	{
        throw std::runtime_error("TpxSerializer unsupports precision: " + std::to_string(data_->prec));
	}
	
	return TPR_SUCCESS;
}

static void print_vec(const char *name, float *arr, int len = DIM*DIM)
{
#ifdef _DEBUG
    msg(name);
    for (int i = 0; i < len; i++) fprintf(stderr, "%f ", arr[i]);
    printf("\n");
#endif // _DEBUG
}

bool TprReader::tpr_body()
{
	// read file foramt version of tpr
	if (!tpr_.do_int(&data_->filever)) return TPR_FAILED;
	msg("File Format Version: %d\n", data_->filever);
	/* This is for backward compatibility with development versions 77-79
	 * where the tag was, mistakenly, placed before the generation,
	 * which would cause a segv instead of a proper error message
	 * when reading the topology only from tpx with <77 code.
	 */
	if (data_->filever >= 77 && data_->filever <= 79)
	{
		char release[MAX_LEN];
		if (!tpr_.xdr_string(release, MAX_LEN)) return TPR_FAILED;
		msg("%s\n", release);
	}
	if (!tpr_.do_int(&data_->vergen)) return TPR_FAILED;
	msg("file generator: %d\n", data_->vergen);

	// release string ?
	if (data_->filever >= 81)
	{
		char buf[MAX_LEN];
        int tempint;
        // 前4个字节未使用
        if (!tpr_.do_int(&tempint)) return TPR_FAILED;

		if (!tpr_.xdr_string(buf, MAX_LEN)) return TPR_FAILED;
		msg("fileTag= %s\n", buf);
	}

	// natoms and ngtc
	if (!tpr_.do_int(&data_->natoms)) return TPR_FAILED;
    INSERT_POS(temperature.g_ngtc); // 温度耦合组数目
	if (!tpr_.do_int(&data_->ngtc)) return TPR_FAILED;
	msg("natoms= %d, ngtc= %d\n", data_->natoms, data_->ngtc);

	// fep state and lambda
	if (data_->filever < 62)
	{
		int tempint;
		float tempreal;
		if (!tpr_.do_int(&tempint)) return TPR_FAILED;
		if (!tpr_.do_real(&tempreal, data_->prec)) return TPR_FAILED;
	}
	if (data_->filever >= 79)
	{
		// fep state
		if (!tpr_.do_int(&data_->fep_state)) return TPR_FAILED;
		msg("fep_state= %d\n", data_->fep_state);
	}
	// lambda 
	if (!tpr_.do_real(&data_->lambda, data_->prec)) return TPR_FAILED;
	msg("lambda= %f\n", data_->lambda);
	// bool type
	if (!tpr_.do_bool(&data_->bIr)) return TPR_FAILED;
	if (!tpr_.do_bool(&data_->bTop)) return TPR_FAILED;
	if (!tpr_.do_bool(&data_->bX)) return TPR_FAILED;
	if (!tpr_.do_bool(&data_->bV)) return TPR_FAILED;
	if (!tpr_.do_bool(&data_->bF)) return TPR_FAILED;
	if (!tpr_.do_bool(&data_->bBox)) return TPR_FAILED;
	msg("bIr= %d, bTop= %d, bX= %d, bV= %d, bF= %d, bBox= %d\n",
		data_->bIr ? 1 : 0,
		data_->bTop ? 1 : 0,
		data_->bX ? 1 : 0,
		data_->bV ? 1 : 0,
		data_->bF ? 1 : 0,
		data_->bBox ? 1 : 0
	);

	if (data_->filever >= tpxv_AddSizeField && data_->vergen >= 27)
	{
		int64_t fsize;
		if (!tpr_.do_int64(&fsize)) return TPR_FAILED;
		msg("Size of tpr body= %lld bytes\n", fsize);
	}
	if (data_->vergen > 28) data_->bIr = false; // This can only happen if TopOnlyOK=TRUE

	// read box size
	if (data_->bBox)
	{
        INSERT_POS(box);
        data_->box.resize(DIM * DIM);
		tpr_.do_vector(data_->box.data(), 9, data_->prec);
        print_vec("box= ", data_->box.data());

		// Relative box vectors characteristic of the box shape, used to to preserve that box shape
		if (data_->filever >= 51)
		{
            INSERT_POS(press.box_rel);
			float box_rel[9] = { 0 };
			tpr_.do_vector(box_rel, 9, data_->prec);
            print_vec("box_rel= ", box_rel);
		}

		// Box velocities for Parrinello-Rahman P-coupling
		float boxv[9] = { 0 };
		tpr_.do_vector(boxv, 9, data_->prec);
        print_vec("boxv= ", boxv);

		if (data_->filever < 56)
		{
			float dump[9] = { 0 };
			tpr_.do_vector(dump, 9, data_->prec);
		}
	}

	// 温度耦合组
	if (data_->ngtc > 0)
	{
        std::vector<float> temparr(data_->ngtc);
		if (data_->filever < 69)
		{
			if (!tpr_.do_vector(temparr.data(), data_->ngtc, data_->prec)) return TPR_FAILED;
		}
		//These used to be the Berendsen tcoupl_lambda's
		if(!tpr_.do_vector(temparr.data(), data_->ngtc, data_->prec)) return TPR_FAILED;
	}

	return TPR_SUCCESS;
}

bool TprReader::tpr_mtop()
{
	//do_mtop starts here, which starts by reading the symtab (do_symtab)
    int symtablen;
	if (!tpr_.do_int(&symtablen)) return TPR_FAILED;
    msg("symtablen= %d\n", symtablen);
    data_->symtab.resize(symtablen * SAVELEN, '\0'); // clear zero

	// 原子类型名称和组名
	for (int i = 0; i < symtablen; i++)
	{
		if (!tpr_.do_string(&data_->symtab[SAVELEN * i], data_->vergen)) return TPR_FAILED;

		// print symb
		if constexpr (0)
		{
			const char* start = &data_->symtab[SAVELEN * i];
			while (*start)
			{
				printf("%c", *start++);
			}
			printf("\n");
		}
	}

	// temp int
	int tempint;
	if (!tpr_.do_int(&tempint)) return TPR_FAILED;
	msg("tempint= %d\n", tempint);

	// read forcefiled parameters
	if (!tpr_readff()) return TPR_FAILED;

    // read type of molecules
    if (!tpr_.do_int(&data_->nmoltypes)) return TPR_FAILED;
    msg("nmoltypes= %d\n", data_->nmoltypes);
    if (!do_atoms()) return TPR_FAILED;

    
    // 保存分子和原子信息到atoms结构体中
    data_->atoms.atomname.resize(data_->natoms);
    data_->atoms.resname.resize(data_->natoms);
    data_->atoms.atomtypename.resize(data_->natoms);
    data_->atoms.resid.resize(data_->natoms);
    data_->atoms.mass.resize(data_->natoms);
    data_->atoms.charge.resize(data_->natoms);
    data_->atoms.atomnumber.resize(data_->natoms);
    data_->atoms.type.resize(data_->natoms);
    unsigned int idx = 0;
    int startedresindex = 1;
    for (int i = 0; i < data_->nmolblock; i++)
    {
        int m = data_->molbtype[i];
        for (int j = 0; j < data_->molbnmol[i]; j++)
        {
            std::set<int> residx;
            for (int k = 0; k < data_->molbnatoms[i]; k++)
            {
                int resind = data_->resids[m][k];
                data_->atoms.atomname[idx]  = &data_->symtab[SAVELEN * data_->atomnameids[m][k]];
                data_->atoms.resname[idx]   = &data_->symtab[SAVELEN * data_->resnames[m][resind]];
                data_->atoms.atomtypename[idx] = &data_->symtab[SAVELEN * data_->atomtypeids[m][k]];

                // 此处的残基编号有问题，当tpr中不连续时候处理不了
                residx.insert(resind);
                data_->atoms.resid[idx]     = resind + startedresindex;

                data_->atoms.mass[idx]      = data_->masses[m][k];
                data_->atoms.charge[idx]    = data_->charges[m][k];
                data_->atoms.atomnumber[idx]= data_->atomicnumbers[m][k];
                data_->atoms.type[idx]      = data_->types[m][k];
                idx++;
            }
            startedresindex += static_cast<int>(residx.size());
        }
    }

	return TPR_SUCCESS;
}

bool TprReader::tpr_xvf()
{
    if (data_->bX)
    {
        INSERT_POS(x);
        data_->atoms.x.resize(data_->natoms * DIM);// 3N 
        if (!tpr_.do_vector(data_->atoms.x.data(), data_->natoms * DIM, data_->prec)) return TPR_FAILED;
    }
    if (data_->bV)
    {
        INSERT_POS(v);
        data_->atoms.v.resize(data_->natoms * DIM);// 3N 
        if (!tpr_.do_vector(data_->atoms.v.data(), data_->natoms * DIM, data_->prec)) return TPR_FAILED;
    }
    if (data_->bF)
    {
        INSERT_POS(f);
        data_->atoms.f.resize(data_->natoms * DIM);// 3N 
        if (!tpr_.do_vector(data_->atoms.f.data(), data_->natoms * DIM, data_->prec)) return TPR_FAILED;
    }

    // write a gro
    if (bGRO_ && data_->bX)
    {
        FILE* fp = fopen("dump.gro", "w");

        fprintf(fp, "MOL\n%d\n", data_->natoms);
        for (int i = 0; i < data_->natoms; i++)
        {
            fprintf(fp, "%5d%-5s%5s%5d%8.3f%8.3f%8.3f",
                data_->atoms.resid[i] % 100000, data_->atoms.resname[i].c_str(), data_->atoms.atomname[i].c_str(), (i + 1) % 100000,
                data_->atoms.x[3L * i + 0], data_->atoms.x[3L * i + 1], data_->atoms.x[3L * i + 2]);

            if (data_->bV)
            {
                fprintf(fp, "%8.4f%8.4f%8.4f", data_->atoms.v[3L * i + 0], data_->atoms.v[3L * i + 1], data_->atoms.v[3L * i + 2]);
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "%10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f %10.5f\n",
            data_->box[0], data_->box[4], data_->box[8],
            data_->box[1], data_->box[2], data_->box[3],
            data_->box[5], data_->box[6], data_->box[7]);
        fclose(fp);
    }

    return TPR_SUCCESS;
}

bool TprReader::tpr_chargemass()
{
    if (bCharge_)
    {
        FILE* fp = fopen("chgmass.dat", "w");
        for (int i = 0; i < data_->atoms.atomname.size(); i++)
        {
            fprintf(fp, "%5s %10.6f %10.6f\n", data_->atoms.atomname[i].c_str(),
                data_->atoms.charge[i], data_->atoms.mass[i]);
        }
        fclose(fp);
    }
    return TPR_SUCCESS;
}

bool TprReader::tpr_bonds()
{
    // bonds type
    const int interactions[] = {
        F_BONDS, F_G96BONDS, F_MORSE, F_CUBICBONDS, F_CONNBONDS, F_HARMONIC, F_FENEBONDS,
        F_CONSTR, F_CONSTRNC, F_TABBONDS, F_TABBONDSNC, F_SETTLE
    };
    constexpr int nBonds = asize(interactions);

    int aoffset = 0;
    for (int i = 0; i < data_->nmolblock; i++)
    {
        int mtype = data_->molbtype[i];
        for (int j = 0; j < data_->molbnmol[i]; j++)
        {
            for (int k = 0; k < nBonds; k++)
            {
                int ftype = interactions[k];
                // settle algorithm for water molecules nspace=4
                const int nspace = (ftype == F_SETTLE) ? 4 : 3;

                for (int m = 0; m < data_->ilist.nr[ftype][mtype] / nspace; m++)
                {
                    // the id of type
                    int itype = data_->ilist.interactionlist[ftype][mtype][nspace * m];

                    // ffparameters
                    auto param = get_bond_type(ftype, &iparams_[itype]);

                    if (ftype == F_SETTLE)
                    {
                        // doh, dHH, indx 0 1 2
                        data_->bonds.emplace_back(1 + aoffset, 2 + aoffset, param.first, param.second);
                        data_->bonds.emplace_back(1 + aoffset, 3 + aoffset, param.first, param.second);
                    }
                    else
                    {
                        int a = nspace * m + 1;
                        int b = nspace * m + 2;
                        data_->bonds.emplace_back(
                            1 + data_->ilist.interactionlist[ftype][mtype][a] + aoffset,
                            1 + data_->ilist.interactionlist[ftype][mtype][b] + aoffset,
                            param.first, param.second
                        );
                    }
                }
            }
            aoffset += data_->atomsinmol[mtype];
        }
    }


    // inter-molecular bonds
    if (data_->bInter)
    {
        // use global atom index, No F_SETTLE
        for (int k = 0; k < nBonds - 1; k++)
        {
            int nameInter = interactions[k];
            for (int m = 0; m < data_->inter_molecular_ilist.nr[nameInter][0] / 3; m++)
            {
                int itype = data_->inter_molecular_ilist.interactionlist[nameInter][0][3 * m];
                int a = 3 * m + 1;
                int b = 3 * m + 2;
                auto param = get_bond_type(nameInter, &iparams_[itype]);
                data_->bonds.emplace_back(
                    1 + data_->inter_molecular_ilist.interactionlist[nameInter][0][a],
                    1 + data_->inter_molecular_ilist.interactionlist[nameInter][0][b],
                    param.first, param.second
                );
            }
        }
    }

    // 成键排序
    std::sort(data_->bonds.begin(), data_->bonds.end(),
        [](const Bonded& lhs, const Bonded& rhs)
        {
            return std::tie(lhs.a, lhs.b) < std::tie(rhs.a, rhs.b);
        }
    );


    // write a mol2 format
    if (bMol2_)
    {
        FILE* fp = fopen("dump.mol2", "w");
        fprintf(fp, "@<TRIPOS>MOLECULE\nMOL\n%d %d 1 0 0\nSMALL\nUSER_CHARGES\n\n\n@<TRIPOS>ATOM\n", data_->natoms, (int)(data_->bonds.size()));
        for (int i = 0; i < data_->natoms; i++)
        {
            fprintf(fp, "%3d %5s %8.4f %8.4f %8.4f %c %5d %5s %8.4f\n",
                i + 1, data_->atoms.atomname[i].c_str(),
                data_->atoms.x[3L * i + 0] * 10.0, data_->atoms.x[3L * i + 1] * 10.0, data_->atoms.x[3L * i + 2] * 10.0,
                data_->atoms.atomname[i].c_str()[0], 1, data_->atoms.resname[i].c_str(), data_->atoms.charge[i]);
        }
        fprintf(fp, "@<TRIPOS>BOND\n");
        int i = 1;
        for (auto& bond : data_->bonds)
        {
            fprintf(fp, "%5d %5d %5d %5d\n", i++, bond.a, bond.b, 1);
        }
        fclose(fp);
    }

    return TPR_SUCCESS;
}

bool TprReader::tpr_angles()
{
    // angles type, drop F_SETTLE because angle from bonds
    const int interactions[] = {
        F_ANGLES, F_G96ANGLES, F_CROSS_BOND_BONDS, F_CROSS_BOND_ANGLES, F_UREY_BRADLEY,
        F_QUARTIC_ANGLES, F_LINEAR_ANGLES, F_RESTRANGLES, F_TABANGLES //, F_SETTLE
    };
    constexpr int nAngles = asize(interactions);

    int aoffset = 0;
    for (int i = 0; i < data_->nmolblock; i++)
    {
        int mtype = data_->molbtype[i];
        for (int j = 0; j < data_->molbnmol[i]; j++)
        {
            for (int k = 0; k < nAngles; k++)
            {
                int ftype = interactions[k];
            
               // force parametersm, note F_SETTLE no angle information
                for (int m = 0; m < data_->ilist.nr[ftype][mtype] / 4; m++)
                {
                    int itype = data_->ilist.interactionlist[ftype][mtype][4 * m];
                    int a = 4 * m + 1;
                    int b = 4 * m + 2;
                    int c = 4 * m + 3;
            
                    auto param = get_angle_type(ftype, &iparams_[itype]);
                    data_->angles.emplace_back(
                        1 + data_->ilist.interactionlist[ftype][mtype][a] + aoffset,
                        1 + data_->ilist.interactionlist[ftype][mtype][b] + aoffset,
                        1 + data_->ilist.interactionlist[ftype][mtype][c] + aoffset,
                        param.first, param.second
                    );
                }
            }
            aoffset += data_->atomsinmol[mtype];
        }
    }

    // 角度排序
    std::sort(data_->angles.begin(), data_->angles.end(),
        [](const Bonded& lhs, const Bonded& rhs)
        {
            return std::tie(lhs.a, lhs.b, lhs.c) < std::tie(rhs.a, rhs.b, rhs.c);
        }
    );

    // TODO
    // 1. inter-molecular angles, dihedrals, imp...

    return TPR_SUCCESS;
}


bool TprReader::tpr_dihedrals()
{
    // proper dihedral type
    const int interactions[] = { F_PDIHS, F_RBDIHS, F_RESTRDIHS, F_CBTDIHS, F_FOURDIHS, F_TABDIHS };
    constexpr int nDihedrals = asize(interactions);

    int aoffset = 0;
    for (int i = 0; i < data_->nmolblock; i++)
    {
        int mtype = data_->molbtype[i];
        for (int j = 0; j < data_->molbnmol[i]; j++)
        {
            for (int k = 0; k < nDihedrals; k++)
            {
                int ftype = interactions[k];
                for (int m = 0; m < data_->ilist.nr[ftype][mtype] / 5; m++)
                {
                    int itype = data_->ilist.interactionlist[ftype][mtype][5 * m];
                    int a = 5 * m + 1;
                    int b = 5 * m + 2;
                    int c = 5 * m + 3;
                    int d = 5 * m + 4;
                    auto param = get_dihedral_type(ftype, &iparams_[itype]);
                    data_->dihedrals.emplace_back(
                        1 + data_->ilist.interactionlist[ftype][mtype][a] + aoffset,
                        1 + data_->ilist.interactionlist[ftype][mtype][b] + aoffset,
                        1 + data_->ilist.interactionlist[ftype][mtype][c] + aoffset,
                        1 + data_->ilist.interactionlist[ftype][mtype][d] + aoffset,
                        param.first, param.second
                    );
                }
            }
            aoffset += data_->atomsinmol[mtype];
        }
    }

    // improper dihedral type
    const int interactions_imp[] = { F_IDIHS, F_PIDIHS };
    constexpr int nImproper = asize(interactions_imp);
    aoffset = 0;
    for (int i = 0; i < data_->nmolblock; i++)
    {
        int mtype = data_->molbtype[i];
        for (int j = 0; j < data_->molbnmol[i]; j++)
        {
            for (int k = 0; k < nImproper; k++)
            {
                int ftype = interactions_imp[k];
                for (int m = 0; m < data_->ilist.nr[ftype][mtype] / 5; m++)
                {
                    int itype = data_->ilist.interactionlist[ftype][mtype][5 * m];
                    int a = 5 * m + 1;
                    int b = 5 * m + 2;
                    int c = 5 * m + 3;
                    int d = 5 * m + 4;
                    auto param = get_improper_type(ftype, &iparams_[itype]);
                    data_->impropers.emplace_back(
                        1 + data_->ilist.interactionlist[ftype][mtype][a] + aoffset,
                        1 + data_->ilist.interactionlist[ftype][mtype][b] + aoffset,
                        1 + data_->ilist.interactionlist[ftype][mtype][c] + aoffset,
                        1 + data_->ilist.interactionlist[ftype][mtype][d] + aoffset,
                        param.first, param.second
                    );
                }
            }
            aoffset += data_->atomsinmol[mtype];
        }
    }

    // 二面角排序
    auto dihfunc = [](const Bonded& lhs, const Bonded& rhs)
    {
        return std::tie(lhs.a, lhs.b, lhs.c, lhs.d) < std::tie(rhs.a, rhs.b, rhs.c, rhs.d);
    };
    std::sort(data_->dihedrals.begin(), data_->dihedrals.end(), dihfunc);
    std::sort(data_->impropers.begin(), data_->impropers.end(), dihfunc);
 
    return TPR_SUCCESS;
}

bool TprReader::tpr_nonbonded()
{
    // get LJ
    std::vector<NonBonded> tempLJ;
    for (size_t i = 0; i < functype_.size(); i++)
    {
        if (functype_[i] == F_LJ)
        {
            auto param = get_nonbonded_type(functype_[i], &iparams_[i]);
            tempLJ.emplace_back(param.first, param.second);
        }
    }
    
    int atnr = data_->atnr;
    msg("atnr= %d\n", atnr);
    msg("tempLJ.size()= %d\n", (int)tempLJ.size());
    if (!tempLJ.empty()) {
        myassert(tempLJ.size() == atnr * atnr,
            "Assert failed: The size of tempLJ must be square of data_->atnr");
    }
    data_->atomtypesLJ.resize(atnr);
    // Add tempLJ check, make sure is Not empty << 2024.09.03
    for (int i = 0; i < atnr && !tempLJ.empty(); i++)
    {
        for (int j = 0; j < atnr; j++)
        {
            if (i == j)
            {
                int idx = i * atnr + j;
                data_->atomtypesLJ[i].ifunc = tempLJ[idx].ifunc;
                // convert C6 and C12 to sigma and epsion
                float sigma = 0, epsion = 0;
                auto C6 = tempLJ[idx].ff[0];
                auto C12 = tempLJ[idx].ff[1];
                if (C6 * C12 != 0)
                {
                    // linux gcc without std::powf
                    sigma = static_cast<float>(std::pow(C12 / C6, 1.0 / 6));
                    epsion = C6 * C6 / (4 * C12);
                }
                data_->atomtypesLJ[i].ff = { sigma, epsion };
            }
        }
    }
    
    int aoffset = 0;
    unsigned int idx = 0;
    for (int i = 0; i < data_->nmolblock; i++)
    {
        int mtype = data_->molbtype[i];
        for (int j = 0; j < data_->molbnmol[i]; j++)
        {
            // LJ for each atoms
            for (int k = 0; k < data_->molbnatoms[i]; k++)
            {
                int itype = data_->types[mtype][k];
                data_->ljparams.emplace_back(data_->atomtypesLJ[itype]);
            }

            // [ pairs ] 
            constexpr int ftype = F_LJ14;
            for (int m = 0; m < data_->ilist.nr[ftype][mtype] / 3; m++)
            {
                int itype = data_->ilist.interactionlist[ftype][mtype][3 * m];
                int a = 3 * m + 1;
                int b = 3 * m + 2;
                auto param = get_nonbonded_type(ftype, &iparams_[itype]);
                data_->pairs.emplace_back(
                    1 + data_->ilist.interactionlist[ftype][mtype][a] + aoffset,
                    1 + data_->ilist.interactionlist[ftype][mtype][b] + aoffset,
                    param.first, param.second
                );
            }
            aoffset += data_->atomsinmol[mtype];
        }
    }

    return TPR_SUCCESS;
}


bool TprReader::tpr_readff()
{
    int		            ntypes;
	double              reppow = 12.0;
	float	            fudge = 0.5;

	if (!tpr_.do_int(&data_->atnr)) return TPR_FAILED;
	if (!tpr_.do_int(&ntypes)) return TPR_FAILED;
	msg("ntypes= %d\n", ntypes);

	// 函数类型
    functype_.resize(ntypes);
	for (int i = 0; i < ntypes; i++)
	{
		if (!tpr_.do_int(&functype_[i])) return TPR_FAILED;
	}
	if (data_->filever >= 66) if (!tpr_.do_double(&reppow)) return TPR_FAILED;
	if (!tpr_.do_real(&fudge, data_->prec)) return TPR_FAILED;
	msg("fudge= %f\n", fudge);
	
	// 调整所有函数类型
    iparams_.resize(ntypes);
	for (int i = 0; i < ntypes; i++)
	{
		for (int j = 0; j < NFTUPD; j++)
		{
			if (data_->filever < ftupd[j].fvnr && functype_[i] >= ftupd[j].ftype)
			{
				functype_[i] += 1;
			}
		}
		// 读力场参数
		if (!do_iparams(functype_[i], &iparams_[i], data_->filever, data_->prec)) return TPR_FAILED;
	}

	return TPR_SUCCESS;
}

bool TprReader::do_iparams(int ftype, t_iparams* iparams, int filever, int prec)
{
    int         idum;
    float       rdum;

    switch (ftype)
    {
    case F_ANGLES:
    case F_G96ANGLES:
    case F_BONDS:
    case F_G96BONDS:
    case F_HARMONIC:
    case F_IDIHS:
        tpr_.do_real(&iparams->harmonic.rA, prec);
        tpr_.do_real(&iparams->harmonic.krA, prec);
        tpr_.do_real(&iparams->harmonic.rB, prec);
        tpr_.do_real(&iparams->harmonic.krB, prec);
        if ((ftype == F_ANGRES || ftype == F_ANGRESZ))
        {
            /* Correct incorrect storage of parameters */
            iparams->pdihs.phiB = iparams->pdihs.phiA;
            iparams->pdihs.cpB = iparams->pdihs.cpA;
        }
        break;
    case F_RESTRANGLES:
        tpr_.do_real(&iparams->harmonic.rA, prec);
        tpr_.do_real(&iparams->harmonic.krA, prec);
        break;
    case F_LINEAR_ANGLES:
        tpr_.do_real(&iparams->linangle.klinA, prec);
        tpr_.do_real(&iparams->linangle.aA, prec);
        tpr_.do_real(&iparams->linangle.klinB, prec);
        tpr_.do_real(&iparams->linangle.aB, prec);

        break;
    case F_FENEBONDS:
        tpr_.do_real(&iparams->fene.bm, prec);
        tpr_.do_real(&iparams->fene.kb, prec);
        break;

    case F_RESTRBONDS:
        tpr_.do_real(&iparams->restraint.lowA, prec);
        tpr_.do_real(&iparams->restraint.up1A, prec);
        tpr_.do_real(&iparams->restraint.up2A, prec);
        tpr_.do_real(&iparams->restraint.kA, prec);
        tpr_.do_real(&iparams->restraint.lowB, prec);
        tpr_.do_real(&iparams->restraint.up1B, prec);
        tpr_.do_real(&iparams->restraint.up2B, prec);
        tpr_.do_real(&iparams->restraint.kB, prec);
        break;
    case F_TABBONDS:
    case F_TABBONDSNC:
    case F_TABANGLES:
    case F_TABDIHS:
        tpr_.do_real(&iparams->tab.kA, prec);
        tpr_.do_int(&iparams->tab.table);
        tpr_.do_real(&iparams->tab.kB, prec);
        break;
    case F_CROSS_BOND_BONDS:
        tpr_.do_real(&iparams->cross_bb.r1e, prec);
        tpr_.do_real(&iparams->cross_bb.r2e, prec);
        tpr_.do_real(&iparams->cross_bb.krr, prec);
        break;
    case F_CROSS_BOND_ANGLES:
        tpr_.do_real(&iparams->cross_ba.r1e, prec);
        tpr_.do_real(&iparams->cross_ba.r2e, prec);
        tpr_.do_real(&iparams->cross_ba.r3e, prec);
        tpr_.do_real(&iparams->cross_ba.krt, prec);
        break;
    case F_UREY_BRADLEY:
        tpr_.do_real(&iparams->u_b.thetaA, prec);
        tpr_.do_real(&iparams->u_b.kthetaA, prec);
        tpr_.do_real(&iparams->u_b.r13A, prec);
        tpr_.do_real(&iparams->u_b.kUBA, prec);
        if (filever >= 79)
        {
            tpr_.do_real(&iparams->u_b.thetaB, prec);
            tpr_.do_real(&iparams->u_b.kthetaB, prec);
            tpr_.do_real(&iparams->u_b.r13B, prec);
            tpr_.do_real(&iparams->u_b.kUBB, prec);
        }
        else
        {
            iparams->u_b.thetaB = iparams->u_b.thetaA;
            iparams->u_b.kthetaB = iparams->u_b.kthetaA;
            iparams->u_b.r13B = iparams->u_b.r13A;
            iparams->u_b.kUBB = iparams->u_b.kUBA;
        }
        break;
    case F_QUARTIC_ANGLES:
        tpr_.do_real(&iparams->qangle.theta, prec);
        tpr_.do_vector(iparams->qangle.c, 5, data_->prec);
        break;
    case F_BHAM:
        tpr_.do_real(&iparams->bham.a, prec);
        tpr_.do_real(&iparams->bham.b, prec);
        tpr_.do_real(&iparams->bham.c, prec);
        break;
    case F_MORSE:
        tpr_.do_real(&iparams->morse.b0A, prec);
        tpr_.do_real(&iparams->morse.cbA, prec);
        tpr_.do_real(&iparams->morse.betaA, prec);
        if (filever >= 79)
        {
            tpr_.do_real(&iparams->morse.b0B, prec);
            tpr_.do_real(&iparams->morse.cbB, prec);
            tpr_.do_real(&iparams->morse.betaB, prec);
        }
        else
        {
            iparams->morse.b0B = iparams->morse.b0A;
            iparams->morse.cbB = iparams->morse.cbA;
            iparams->morse.betaB = iparams->morse.betaA;
        }
        break;
    case F_CUBICBONDS:
        tpr_.do_real(&iparams->cubic.b0, prec);
        tpr_.do_real(&iparams->cubic.kb, prec);
        tpr_.do_real(&iparams->cubic.kcub, prec);
        break;
    case F_CONNBONDS: break;
    case F_POLARIZATION: 
        tpr_.do_real(&iparams->polarize.alpha, prec); 
        break;
    case F_ANHARM_POL:
        tpr_.do_real(&iparams->anharm_polarize.alpha, prec);
        tpr_.do_real(&iparams->anharm_polarize.drcut, prec);
        tpr_.do_real(&iparams->anharm_polarize.khyp, prec);
        break;
    case F_WATER_POL:
        tpr_.do_real(&iparams->wpol.al_x, prec);
        tpr_.do_real(&iparams->wpol.al_y, prec);
        tpr_.do_real(&iparams->wpol.al_z, prec);
        tpr_.do_real(&iparams->wpol.rOH, prec);
        tpr_.do_real(&iparams->wpol.rHH, prec);
        tpr_.do_real(&iparams->wpol.rOD, prec);
        break;
    case F_THOLE_POL:
        tpr_.do_real(&iparams->thole.a, prec);
        tpr_.do_real(&iparams->thole.alpha1, prec);
        tpr_.do_real(&iparams->thole.alpha2, prec);
        if (filever < tpxv_RemoveTholeRfac)
        {
            float noRfac = 0;
            tpr_.do_real(&noRfac, prec);
        }

        break;
    case F_LJ:
        tpr_.do_real(&iparams->lj.c6, prec);
        tpr_.do_real(&iparams->lj.c12, prec);
        break;
    case F_LJ14:
        tpr_.do_real(&iparams->lj14.c6A, prec);
        tpr_.do_real(&iparams->lj14.c12A, prec);
        tpr_.do_real(&iparams->lj14.c6B, prec);
        tpr_.do_real(&iparams->lj14.c12B, prec);
        break;
    case F_LJC14_Q:
        tpr_.do_real(&iparams->ljc14.fqq, prec);
        tpr_.do_real(&iparams->ljc14.qi, prec);
        tpr_.do_real(&iparams->ljc14.qj, prec);
        tpr_.do_real(&iparams->ljc14.c6, prec);
        tpr_.do_real(&iparams->ljc14.c12, prec);
        break;
    case F_LJC_PAIRS_NB:
        tpr_.do_real(&iparams->ljcnb.qi, prec);
        tpr_.do_real(&iparams->ljcnb.qj, prec);
        tpr_.do_real(&iparams->ljcnb.c6, prec);
        tpr_.do_real(&iparams->ljcnb.c12, prec);
        break;
    case F_PDIHS:
    case F_PIDIHS:
    case F_ANGRES:
    case F_ANGRESZ:
        tpr_.do_real(&iparams->pdihs.phiA, prec);
        tpr_.do_real(&iparams->pdihs.cpA, prec);
        tpr_.do_real(&iparams->pdihs.phiB, prec);
        tpr_.do_real(&iparams->pdihs.cpB, prec);
        tpr_.do_int(&iparams->pdihs.mult);
        break;
    case F_RESTRDIHS:
        tpr_.do_real(&iparams->pdihs.phiA, prec);
        tpr_.do_real(&iparams->pdihs.cpA, prec);
        break;
    case F_DISRES:
        tpr_.do_int(&iparams->disres.label);
        tpr_.do_int(&iparams->disres.type);
        tpr_.do_real(&iparams->disres.low, prec);
        tpr_.do_real(&iparams->disres.up1, prec);
        tpr_.do_real(&iparams->disres.up2, prec);
        tpr_.do_real(&iparams->disres.kfac, prec);
        break;
    case F_ORIRES:
        tpr_.do_int(&iparams->orires.ex);
        tpr_.do_int(&iparams->orires.label);
        tpr_.do_int(&iparams->orires.power);
        tpr_.do_real(&iparams->orires.c, prec);
        tpr_.do_real(&iparams->orires.obs, prec);
        tpr_.do_real(&iparams->orires.kfac, prec);
        break;
    case F_DIHRES:
        if (filever < 82)
        {
            tpr_.do_int(&idum);
            tpr_.do_int(&idum);
        }
        tpr_.do_real(&iparams->dihres.phiA, prec);
        tpr_.do_real(&iparams->dihres.dphiA, prec);
        tpr_.do_real(&iparams->dihres.kfacA, prec);
        if (filever >= 82)
        {
            tpr_.do_real(&iparams->dihres.phiB, prec);
            tpr_.do_real(&iparams->dihres.dphiB, prec);
            tpr_.do_real(&iparams->dihres.kfacB, prec);
        }
        else
        {
            iparams->dihres.phiB = iparams->dihres.phiA;
            iparams->dihres.dphiB = iparams->dihres.dphiA;
            iparams->dihres.kfacB = iparams->dihres.kfacA;
        }
        break;
    case F_POSRES:
        tpr_.do_vector(iparams->posres.pos0A, DIM, data_->prec);
        tpr_.do_vector(iparams->posres.fcA, DIM, data_->prec);
        tpr_.do_vector(iparams->posres.pos0B, DIM, data_->prec);
        tpr_.do_vector(iparams->posres.fcB, DIM, data_->prec);
        break;
    case F_FBPOSRES:
        tpr_.do_int(&iparams->fbposres.geom);
        tpr_.do_vector(iparams->fbposres.pos0, DIM, data_->prec);
        tpr_.do_real(&iparams->fbposres.r, prec);
        tpr_.do_real(&iparams->fbposres.k, prec);
        break;
    case F_CBTDIHS: 
        tpr_.do_vector(iparams->cbtdihs.cbtcA, NR_CBTDIHS, data_->prec);
        break;
    case F_RBDIHS:
        // Fall-through intended
    case F_FOURDIHS:
        /* Fourier dihedrals are internally represented
         * as Ryckaert-Bellemans since those are faster to compute.
         */
        tpr_.do_vector(iparams->rbdihs.rbcA, NR_RBDIHS, data_->prec);
        tpr_.do_vector(iparams->rbdihs.rbcB, NR_RBDIHS, data_->prec);
        break;
    case F_CONSTR:
    case F_CONSTRNC:
        tpr_.do_real(&iparams->constr.dA, prec);
        tpr_.do_real(&iparams->constr.dB, prec);
        break;
    case F_SETTLE:
        tpr_.do_real(&iparams->settle.doh, prec);
        tpr_.do_real(&iparams->settle.dhh, prec);
        break;
    case F_VSITE1: 
        break; // VSite1 has 0 parameters
    case F_VSITE2:
    case F_VSITE2FD:
        tpr_.do_real(&iparams->vsite.a, prec); 
        break;
    case F_VSITE3:
    case F_VSITE3FD:
    case F_VSITE3FAD:
        tpr_.do_real(&iparams->vsite.a, prec);
        tpr_.do_real(&iparams->vsite.b, prec);
        break;
    case F_VSITE3OUT:
    case F_VSITE4FD:
    case F_VSITE4FDN:
        tpr_.do_real(&iparams->vsite.a, prec);
        tpr_.do_real(&iparams->vsite.b, prec);
        tpr_.do_real(&iparams->vsite.c, prec);
        break;
    case F_VSITEN:
        tpr_.do_int(&iparams->vsiten.n);
        tpr_.do_real(&iparams->vsiten.a, prec);
        break;
    case F_GB12_NOLONGERUSED:
    case F_GB13_NOLONGERUSED:
    case F_GB14_NOLONGERUSED:
        // Implicit solvent parameters can still be read, but never used
        //if (serializer->reading())
        {
            if (filever < 68)
            {
                tpr_.do_real(&rdum, prec);
                tpr_.do_real(&rdum, prec);
                tpr_.do_real(&rdum, prec);
                tpr_.do_real(&rdum, prec);
            }
            if (filever < tpxv_RemoveImplicitSolvation)
            {
                tpr_.do_real(&rdum, prec);
                tpr_.do_real(&rdum, prec);
                tpr_.do_real(&rdum, prec);
                tpr_.do_real(&rdum, prec);
                tpr_.do_real(&rdum, prec);
            }
        }
        break;
    case F_CMAP:
        tpr_.do_int(&iparams->cmap.cmapA);
        tpr_.do_int(&iparams->cmap.cmapB);
        break;
    default:
        msg("Unknown function type %d", ftype);
    }

	return TPR_SUCCESS;
}

bool TprReader::do_atoms()
{
    const int       n = data_->nmoltypes;
    float           rdum;
    int             idum, idum2;
    unsigned char   ucdum;
    unsigned short  usdum;

    data_->atomsinmol.resize(n);
    data_->molnames.resize(n);
    data_->resinmol.resize(n);
    data_->charges.resize(n);
    data_->masses.resize(n);
    data_->ptypes.resize(n);
    data_->types.resize(n);
    data_->resids.resize(n);
    data_->atomnameids.resize(n);
    data_->atomtypeids.resize(n);
    data_->atomicnumbers.resize(n);
    data_->resnames.resize(n);
    for (int i = 0; i < F_NRE; i++)
    {
        data_->ilist.interactionlist[i].resize(n);
        data_->ilist.nr[i].resize(n);
    }

    // read each mol
    for (int i = 0; i < n; i++)
    {
        // 分子名称长度
        if (!tpr_.do_int(&data_->molnames[i])) return TPR_FAILED;
        msg("data_->molnames[i]= %d\n", data_->molnames[i]);

        // 每个moltype的原子数目
        if (!tpr_.do_int(&data_->atomsinmol[i])) return TPR_FAILED;
        // 每个moltype的残基数目
        if (!tpr_.do_int(&data_->resinmol[i])) return TPR_FAILED;

        // allocate for 2D vector
        data_->charges[i].resize(data_->atomsinmol[i]);
        data_->masses[i].resize(data_->atomsinmol[i]);
        data_->types[i].resize(data_->atomsinmol[i]);
        data_->ptypes[i].resize(data_->atomsinmol[i]);
        data_->resids[i].resize(data_->atomsinmol[i]);
        // initial -1 because data_->filever >= 52
        data_->atomicnumbers[i].resize(data_->atomsinmol[i], -1); 
        for (int j = 0; j < data_->atomsinmol[i]; j++)
        {
            // 读原子质量，电荷
            if (!tpr_.do_real(&data_->masses[i][j], data_->prec)) return TPR_FAILED;
            if (!tpr_.do_real(&data_->charges[i][j], data_->prec)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED; //mB
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED; //qB

            // for gmx>=2020，short只读2字节
            if (!tpr_.do_ushort(&data_->types[i][j], data_->vergen)) return TPR_FAILED;
            if (!tpr_.do_ushort(&usdum, data_->vergen)) return TPR_FAILED;

            if (!tpr_.do_int(&data_->ptypes[i][j])) return TPR_FAILED;
            if (!tpr_.do_int(&data_->resids[i][j])) return TPR_FAILED;
            if (data_->filever >= 52)
            {
                if (!tpr_.do_int(&data_->atomicnumbers[i][j])) return TPR_FAILED;
            }
            //msg("data_->types[i][j]= %d\n", data_->types[i][j]);
            //msg("data_->ptypes[i][j]= %d\n", data_->ptypes[i][j]);
            //msg("data_->resids[i][j]= %d\n", data_->resids[i][j]);
            //msg("data_->atomicnumbers[i][j]= %d\n", data_->atomicnumbers[i][j]);
        }

        // allocated for 2D vector
        data_->atomnameids[i].resize(data_->atomsinmol[i]);
        data_->atomtypeids[i].resize(data_->atomsinmol[i]);
        if (!tpr_.do_vector(data_->atomnameids[i].data(), data_->atomsinmol[i], data_->prec)) return TPR_FAILED;
        if (!tpr_.do_vector(data_->atomtypeids[i].data(), data_->atomsinmol[i], data_->prec)) return TPR_FAILED;
        // typeB
        for (int j = 0; j < data_->atomsinmol[i]; j++)
        {
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
        }

        //read residues
        data_->resnames[i].resize(data_->resinmol[i]);
        for (int j = 0; j < data_->resinmol[i]; j++)
        {
            if (!tpr_.do_int(&data_->resnames[i][j])) return TPR_FAILED;
            //msg("data_->resnames[i][j]= %d\n", data_->resnames[i][j]);

            if (data_->filever >= 63)
            {
                // true Residue number 
                if (!tpr_.do_int(&idum)) return TPR_FAILED;
                data_->trueresids.push_back(idum);

                // gmx >= 2020, 只读一字节
                if (!tpr_.do_uchar(&ucdum, data_->vergen)) return TPR_FAILED;
            }
            else
            {
                data_->resnames[i][j] += 1;
            }
        }

        // do_ilists
        msg("do_ilists\n");
        if (!do_ilists(i, data_->ilist.nr, data_->ilist.interactionlist)) return TPR_FAILED;

        // charge groups parts
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        std::vector<int> temp(idum + 1); // need +1
        if (!tpr_.do_vector(temp.data(), idum + 1, data_->prec)) return TPR_FAILED;
        // doListOfLists
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        if (!tpr_.do_int(&idum2)) return TPR_FAILED;
        temp.resize(idum + 1); // need +1
        if (!tpr_.do_vector(temp.data(), idum + 1, data_->prec)) return TPR_FAILED;
        temp.resize(idum2); // not need +1
        if (!tpr_.do_vector(temp.data(), idum2, data_->prec)) return TPR_FAILED;
    } 


    // do molblock
    if (!tpr_.do_int(&data_->nmolblock)) return TPR_FAILED;
    msg("nmolblock= %d\n", data_->nmolblock);
    data_->molbtype.resize(data_->nmolblock);
    data_->molbnmol.resize(data_->nmolblock);
    data_->molbnatoms.resize(data_->nmolblock);
    for (int i = 0; i < data_->nmolblock; i++)
    {
        if (!tpr_.do_int(&data_->molbtype[i])) return TPR_FAILED;
        if (!tpr_.do_int(&data_->molbnmol[i])) return TPR_FAILED;
        if (!tpr_.do_int(&data_->molbnatoms[i])) return TPR_FAILED;
        msg("data_->molbtype[i]= %d\n", data_->molbtype[i]);
        msg("data_->molbnmol[i]= %d\n", data_->molbnmol[i]);
        msg("data_->molbnatoms[i]= %d\n", data_->molbnatoms[i]);

        // posres
        if (!tpr_.do_int(&idum)) return TPR_FAILED;  //posres_xA
        msg("posres_xA= %d\n", idum);
        if (idum > 0)
        {
            std::vector<float> temp(idum * DIM);
            if (!tpr_.do_vector(temp.data(), idum * DIM, data_->prec)) return TPR_FAILED;
        }

        if (!tpr_.do_int(&idum)) return TPR_FAILED;  //posres_xB
        msg("posres_xB= %d\n", idum);
        if (idum > 0)
        {
            std::vector<float> temp(idum * DIM);
            if (!tpr_.do_vector(temp.data(), idum * DIM, data_->prec)) return TPR_FAILED;
        }
    }
    // 体系全局原子数
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    msg("The number of atoms= %d\n", idum);
    
    // inter-molecularbonds
    if (data_->filever >= tpxv_IntermolecularBondeds)
    {
        // for gmx>=2020, read 1 byte
        if (!tpr_.do_bool(&data_->bInter, data_->vergen))  return TPR_FAILED;
        msg("bInter= %d\n", data_->bInter ? 1 : 0);
        if (data_->bInter)
        {
            // allocated
            for (int i = 0; i < F_NRE; i++)
            {
                data_->inter_molecular_ilist.interactionlist[i].resize(1);
                data_->inter_molecular_ilist.nr[i].resize(1);
            }
            do_ilists(0, data_->inter_molecular_ilist.nr, data_->inter_molecular_ilist.interactionlist);
        }
    }

    if (data_->filever < tpxv_RemoveAtomtypes)
    {
        if (!do_atomtypes()) return TPR_FAILED;
    }

    if (data_->filever >= 65)
    {
        if (!do_cmap()) return TPR_FAILED;
    }

    if (!do_groups()) return TPR_FAILED;

    if (data_->filever >= tpxv_StoreNonBondedInteractionExclusionGroup)
    {
        int64_t intermolecularExclusionGroupSize;
        if (!tpr_.do_int64(&intermolecularExclusionGroupSize)) return TPR_FAILED;
        std::vector<int> temp(intermolecularExclusionGroupSize);
        if (!tpr_.do_vector(temp.data(), static_cast<int>(intermolecularExclusionGroupSize))) return TPR_FAILED;
    }

    return TPR_SUCCESS;
}

bool TprReader::do_atomtypes()
{
    int     nr;

    if (!tpr_.do_int(&nr)) return TPR_FAILED;
    if (data_->filever < tpxv_RemoveImplicitSolvation)
    {
        std::vector<float> temp(nr);
        if (!tpr_.do_vector(temp.data(), nr, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_vector(temp.data(), nr, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_vector(temp.data(), nr, data_->prec)) return TPR_FAILED;
    }

    // read atomtype number [ atomtypes ]
    data_->atoms.atomtypenumber.resize(nr);
    if (!tpr_.do_vector(data_->atoms.atomtypenumber.data(), nr, data_->prec)) return TPR_FAILED;

    if (data_->filever >= 60 && data_->filever < tpxv_RemoveImplicitSolvation)
    {
        std::vector<float> temp(nr);
        if (!tpr_.do_vector(temp.data(), nr, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_vector(temp.data(), nr, data_->prec)) return TPR_FAILED;
    }

    return TPR_SUCCESS;
}

bool TprReader::do_cmap()
{
    int         ngrid, gridspace;
    float       rdum;

    if (!tpr_.do_int(&ngrid)) return TPR_FAILED;
    if (!tpr_.do_int(&gridspace)) return TPR_FAILED;
    msg("ngrid= %d, gridspace= %d\n", ngrid, gridspace);

    for (int i = 0; i < ngrid * gridspace * gridspace; i++)
    {
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    }

    return TPR_SUCCESS;
}

bool TprReader::do_ir()
{
    int     idum;
    float   rdum;
    bool    bdum;
    auto    ir = &data_->ir;

    msg("------------------------------\n");
    if (data_->filever >= 53)
    {
        if (!tpr_.do_int(reinterpret_cast<int*>(&ir->pbc))) return TPR_FAILED;
        msg("pbcType= %d\n", static_cast<int>(data_->ir.pbc));
        if (!tpr_.do_bool(&ir->pbcmol, data_->vergen)) return TPR_FAILED;
        msg("pbcmol= %d\n", ir->pbcmol ? 1 : 0);
    }
    // too new tpr have not yet support read ir
    if (data_->vergen > tpx_generation) return TPR_FAILED;

    // integration method
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    msg("integration method= %d\n", idum);
    
    // nsteps
    INSERT_POS(nsteps);
    if (data_->filever >= 62)
    {
        if (!tpr_.do_int64(&data_->ir.nsteps)) return TPR_FAILED;
    }
    else
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        ir->nsteps = static_cast<int64_t>(idum); // int to int64
    }
    msg("nsteps= %lld\n", ir->nsteps);

    if (data_->filever >= 62)
    {
        if (!tpr_.do_int64(&ir->init_step)) return TPR_FAILED;
    }
    else
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        ir->init_step = static_cast<int64_t>(idum); // int to int64
    }
    msg("init_step= %lld\n", ir->init_step);

    if (!tpr_.do_int(&ir->simulation_part)) return TPR_FAILED;
    msg("simulation_part= %d\n", ir->simulation_part);
    
    // 多时步模拟参数,ignor
    if (data_->filever >= tpxv_MTS)
    {
        bool    useMts;
        if (!tpr_.do_bool(&useMts, data_->vergen)) return TPR_FAILED;
        msg("mts= %d\n", useMts ? 1 : 0);
        if (useMts)
        {
            // numLevels
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            int forceGroups, stepFactor;
            for (int i = 0; i < idum; i++)
            {
                if (!tpr_.do_int(&forceGroups)) return TPR_FAILED;
                if (!tpr_.do_int(&stepFactor)) return TPR_FAILED;
            }
        }
    }
    
    if (data_->filever >= tpxv_MassRepartitioning)
    {
        float massRepartitionFactor;
        if (!tpr_.do_real(&massRepartitionFactor, data_->prec)) return TPR_FAILED;
        msg("massRepartitionFactor= %f\n", massRepartitionFactor);
    }

    if (data_->filever >= tpxv_EnsembleTemperature)
    {
        //ensembleTemperatureSetting
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        //ensembleTemperature
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        msg("ensembleTemperature= %f\n", rdum);
    }

    //nstcalcenergy
    if (data_->filever >= 67)
    {
        INSERT_POS(integer.nstcalcenergy);
        if (!tpr_.do_int(&ir->nstcalcenergy)) return TPR_FAILED;
    }
    else
    {
        ir->nstcalcenergy = 1;
    }
    msg("nstcalcenergy= %d\n", ir->nstcalcenergy);

    if (data_->filever >= 81)
    {
        // cutoff_scheme
        if (!tpr_.do_int(&ir->cutoff_scheme)) return TPR_FAILED;
    }
    else
    {
        ir->cutoff_scheme = 1; // groups
    }
    msg("cutoff_scheme= %d\n", ir->cutoff_scheme);

    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    msg("ns_type= %d\n", idum);
    if (!tpr_.do_int(&ir->nstlist)) return TPR_FAILED;
    msg("nstlist= %d\n", ir->nstlist);
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    msg("ndelta= %d\n", idum);

    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    msg("rtpi= %f\n", rdum); // test particle radius
    if (!tpr_.do_int(&ir->nstcomm)) return TPR_FAILED;
    msg("nstcomm= %d\n", ir->nstcomm); 
    if (!tpr_.do_int(&ir->comm_mode)) return TPR_FAILED;
    msg("comm_mode= %d\n", ir->comm_mode);

    // ignore nstcheckpoint
    if (data_->filever < tpxv_RemoveObsoleteParameters1)
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
    }
    
    if (!tpr_.do_int(&ir->nstcgsteep)) return TPR_FAILED;
    msg("nstcgsteep= %d\n", ir->nstcgsteep);
    if (!tpr_.do_int(&ir->nbfgscorr)) return TPR_FAILED;
    msg("nbfgscorr= %d\n", ir->nbfgscorr);

    INSERT_POS(integer.nstlog);
    if (!tpr_.do_int(&ir->nstlog)) return TPR_FAILED;
    msg("nstlog= %d\n", ir->nstlog);

    INSERT_POS(integer.nstxout);
    if (!tpr_.do_int(&ir->nstxout)) return TPR_FAILED;
    msg("nstxout= %d\n", ir->nstxout);

    INSERT_POS(integer.nstvout);
    if (!tpr_.do_int(&ir->nstvout)) return TPR_FAILED;
    msg("nstvout= %d\n", ir->nstvout);

    INSERT_POS(integer.nstfout);
    if (!tpr_.do_int(&ir->nstfout)) return TPR_FAILED;
    msg("nstfout= %d\n", ir->nstfout);

    INSERT_POS(integer.nstenergy);
    if (!tpr_.do_int(&ir->nstenergy)) return TPR_FAILED;
    msg("nstenergy= %d\n", ir->nstenergy);

    INSERT_POS(integer.nstxout_compressed);
    if (!tpr_.do_int(&ir->nstxout_compressed)) return TPR_FAILED;
    msg("nstxout_compressed= %d\n", ir->nstxout_compressed);

    if (data_->filever >= 59)
    {
        if (!tpr_.do_double(&ir->init_t)) return TPR_FAILED;

        INSERT_POS(dt); // get dt position
        if (!tpr_.do_double(&ir->dt)) return TPR_FAILED;
    }
    else
    {
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        ir->init_t = static_cast<double>(rdum);

        INSERT_POS(dt); // get dt position
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        ir->dt = static_cast<double>(rdum);
    }
    msg("init_t= %g\n", ir->init_t);
    msg("delta_t= %g (ps)\n", ir->dt);

    if (!tpr_.do_real(&ir->x_compression_precision, data_->prec)) return TPR_FAILED;
    msg("xtc prec= %g\n", ir->x_compression_precision);
    
    if (data_->filever >= 81)
    {
        if (!tpr_.do_real(&ir->verletbuf_tol, data_->prec)) return TPR_FAILED;
    }
    else
    {
        ir->verletbuf_tol = 0.0f;
    }
    msg("tolerance of verlet buffer= %g\n", ir->verletbuf_tol);

    if (data_->filever >= tpxv_VerletBufferPressureTol)
    {
        if (!tpr_.do_real(&ir->verletBufferPressureTolerance, data_->prec)) return TPR_FAILED;
    }
    else
    {
        ir->verletBufferPressureTolerance = -1;
    }
    msg("verletBufferPressureTolerancer= %g\n", ir->verletBufferPressureTolerance);

    if (!tpr_.do_real(&ir->rlist, data_->prec)) return TPR_FAILED;
    msg("rlist= %g\n", ir->rlist);

    // twin-range interactions
    if (data_->filever >= 67 && data_->filever < tpxv_RemoveTwinRange)
    {
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        msg("dummy_rlistlong= %g\n", rdum);
    }

    // 
    if (data_->filever >= 82 && data_->filever != 90)
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        msg("dummy_nstcalclr= %d\n", idum);
    }

    // 静电处理方式
    if (!tpr_.do_int(&ir->coulombtype)) return TPR_FAILED;
    msg("coulombtype= %d\n", ir->coulombtype);
    if (data_->filever >= 81)
    {
        if (!tpr_.do_int(&ir->coulomb_modifier)) return TPR_FAILED;
    }
    else
    {
        ir->coulomb_modifier = ir->cutoff_scheme == 0 ? 1 : 2;
    }
    msg("coulomb_modifier= %d\n", ir->coulomb_modifier);

    if (!tpr_.do_real(&ir->rcoulomb_switch, data_->prec)) return TPR_FAILED;
    msg("rcoulomb_switch= %g\n", ir->rcoulomb_switch);
    if (!tpr_.do_real(&ir->rcoulomb, data_->prec)) return TPR_FAILED;
    msg("rcoulomb= %g\n", ir->rcoulomb);
    if (!tpr_.do_int(&ir->vdwtype)) return TPR_FAILED;
    msg("vdwtype= %d\n", ir->vdwtype);
    if (data_->filever >= 81)
    {
        if (!tpr_.do_int(&ir->vdw_modifier)) return TPR_FAILED;
    }
    else
    {
        ir->vdw_modifier = ir->cutoff_scheme == 0 ? 1 : 2;
    }
    msg("vdw_modifier= %d\n", ir->vdw_modifier);


    if (!tpr_.do_real(&ir->rvdw_switch, data_->prec)) return TPR_FAILED;
    msg("rvdw_switch= %g\n", ir->rvdw_switch);
    if (!tpr_.do_real(&ir->rvdw, data_->prec)) return TPR_FAILED;
    msg("rvdw= %g\n", ir->rvdw);
    if (!tpr_.do_int(&ir->eDispCorr)) return TPR_FAILED;
    msg("eDispCorr= %d\n", ir->eDispCorr);
    if (!tpr_.do_real(&ir->epsilon_r, data_->prec)) return TPR_FAILED;
    msg("epsilon_r= %g\n", ir->epsilon_r);
    if (!tpr_.do_real(&ir->epsilon_rf, data_->prec)) return TPR_FAILED;
    msg("epsilon_rf= %g\n", ir->epsilon_rf);
    if (!tpr_.do_real(&ir->tabext, data_->prec)) return TPR_FAILED;
    msg("tabext= %g\n", ir->tabext);

    // 隐式溶剂部分
    if (data_->filever < tpxv_RemoveImplicitSolvation)
    {
        // int, int, real, real, int
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        ir->implicit_solvent = idum > 0;
    }
    else
    {
        ir->implicit_solvent = false;
    }

    if (data_->filever < tpxv_RemoveImplicitSolvation)
    {
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (data_->filever >= 60)
        {
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
        }
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    }

    if (data_->filever >= 81)
    {
        if (!tpr_.do_real(&ir->fourier_spacing, data_->prec)) return TPR_FAILED;
    }
    else
    {
        ir->fourier_spacing = 0.0;
    }
    if (!tpr_.do_int(&ir->nkx)) return TPR_FAILED;
    if (!tpr_.do_int(&ir->nky)) return TPR_FAILED;
    if (!tpr_.do_int(&ir->nkz)) return TPR_FAILED;
    if (!tpr_.do_int(&ir->pme_order)) return TPR_FAILED;
    if (!tpr_.do_real(&ir->ewald_rtol, data_->prec)) return TPR_FAILED;
    if (data_->filever >= 93)
    {
        if (!tpr_.do_real(&ir->ewald_rtol_lj, data_->prec)) return TPR_FAILED;
    }
    else
    {
        ir->ewald_rtol_lj = ir->ewald_rtol;
    }
    if (!tpr_.do_int(&ir->ewald_geometry)) return TPR_FAILED;
    msg("ewald_geometry= %s\n", ir->ewald_geometry == 0 ? "3D" : "3DC");

    if (!tpr_.do_real(&ir->epsilon_surface, data_->prec)) return TPR_FAILED;
    msg("epsilon_surface= %g\n", ir->epsilon_surface);

    // ignore bOptFFT
    if (data_->filever < tpxv_RemoveObsoleteParameters1)
    {
        if (!tpr_.do_bool(&bdum, data_->vergen)) return  TPR_FAILED;
    }

    if (data_->filever >= 93)
    {
        if (!tpr_.do_int(&ir->ljpme_combination_rule)) return TPR_FAILED;
        msg("ljpme_combination_rule= %s\n", ir->ljpme_combination_rule == 0 ? "Geom" : "LB");
    }
    if (!tpr_.do_bool(&ir->bContinuation, data_->vergen)) return  TPR_FAILED;
    msg("Continuation= %d\n", ir->bContinuation ? 1 : 0);

    // 温度耦合
    INSERT_POS(temperature.etc);
    if (!tpr_.do_int(&ir->etc)) return TPR_FAILED;
    msg("etc= %d\n", ir->etc);

    // bPrintNHChains
    if (data_->filever >= 79)
    {
        if (!tpr_.do_bool(&bdum, data_->vergen)) return  TPR_FAILED;
    }
    if (data_->filever >= 71)
    {
        INSERT_POS(integer.nsttcouple);
        if (!tpr_.do_int(&ir->nsttcouple)) return TPR_FAILED;
    }
    else
    {
        ir->nsttcouple = ir->nstcalcenergy;
    }
    msg("nsttcouple= %d\n", ir->nsttcouple);

    INSERT_POS(press.epc);
    if (!tpr_.do_int(&ir->epc)) return TPR_FAILED;
    msg("epc= %d\n", ir->epc);
    INSERT_POS(press.epct);
    if (!tpr_.do_int(&ir->epct)) return TPR_FAILED;
    msg("epct= %d\n", ir->epct);

    if (data_->filever >= 71)
    {
        INSERT_POS(integer.nstpcouple);
        if (!tpr_.do_int(&ir->nstpcouple)) return TPR_FAILED;
    }
    else
    {
        ir->nstpcouple = ir->nstcalcenergy;
    }
    msg("nstpcouple= %d\n", ir->nstpcouple);

    INSERT_POS(press.tau_p);
    if (!tpr_.do_real(&ir->tau_p, data_->prec)) return TPR_FAILED;
    msg("tau_p= %g\n", ir->tau_p);

    // pressure
    INSERT_POS(press.ref_p);
    if (!tpr_.do_vector(ir->ref_p, 9, data_->prec)) return TPR_FAILED;
    print_vec("ref_p= ", ir->ref_p);
    INSERT_POS(press.compress);
    if (!tpr_.do_vector(ir->compress, 9, data_->prec)) return TPR_FAILED;
    print_vec("compress= ", ir->compress);
    if (!tpr_.do_int(&ir->refcoord_scaling)) return TPR_FAILED;
    msg("refcoord_scaling= %d\n", ir->refcoord_scaling);
    if (!tpr_.do_vector(ir->posres_com, DIM, data_->prec)) return TPR_FAILED;
    print_vec("posres_com= ", ir->posres_com, DIM);
    if (!tpr_.do_vector(ir->posres_comB, DIM, data_->prec)) return TPR_FAILED;
    print_vec("posres_comB= ", ir->posres_comB, DIM);

    //andersen_seed
    if (data_->filever < 79)
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
    }
    
    if (!tpr_.do_real(&ir->shake_tol, data_->prec)) return TPR_FAILED;
    msg("shake_tol= %g\n", ir->shake_tol);

    // 自由能计算部分
    if (!tpr_.do_int(&ir->efep)) return TPR_FAILED;
    if (!do_fepvals())  return TPR_FAILED;

    if (data_->filever >= 79)
    {
        if (!tpr_.do_bool(&ir->bSimTemp, data_->vergen)) return TPR_FAILED;
    }
    else
    {
        ir->bSimTemp = false;
    }
    msg("bSimTemp= %d\n", ir->bSimTemp ? 1 : 0);
    //do_simtempvals
    if (ir->bSimTemp)
    {
        if (data_->filever >= 79)
        {
            //eSimTempScale
            if (!tpr_.do_int(&ir->eSimTempScale)) return TPR_FAILED;
            if (!tpr_.do_real(&ir->simtemp_high, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_real(&ir->simtemp_high, data_->prec)) return TPR_FAILED;

            // The range of temperatures used for simulated tempering
            if (ir->n_lambda > 0)
            {
                std::vector<float> temperatures(ir->n_lambda);
                if (!tpr_.do_vector(temperatures.data(), ir->n_lambda, data_->prec)) return TPR_FAILED;
            }
        }
    }

    if (data_->filever >= 79)
    {
        if (!tpr_.do_bool(&ir->bExpanded, data_->vergen)) return TPR_FAILED;
    }
    else
    {
        ir->bExpanded = false;
    }
    if (ir->bExpanded)
    {
        //do_expandedvals
        if (data_->filever >= 79)
        {
            if (ir->n_lambda > 0)
            {
                std::vector<float> init_lambda_weights(ir->n_lambda);
                if (!tpr_.do_vector(init_lambda_weights.data(), ir->n_lambda, data_->prec)) return TPR_FAILED;
                //bInit_weights
                if (!tpr_.do_bool(&bdum, data_->vergen)) return TPR_FAILED;
            }

            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_bool(&bdum, data_->vergen)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_bool(&bdum, data_->vergen)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        }
    }

    // eDisre, eDisreWeighting
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    if (!tpr_.do_bool(&bdum, data_->vergen)) return TPR_FAILED;
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    if (!tpr_.do_int(&idum)) return TPR_FAILED;

    // ignore dihre_fc
    if (data_->filever < 79)
    {
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    }


    // em_stepsize, em_tol
    if (!tpr_.do_real(&ir->em_stepsize, data_->prec)) return TPR_FAILED;
    msg("em_stepsize= %g\n", ir->em_stepsize);
    if (!tpr_.do_real(&ir->em_tol, data_->prec)) return TPR_FAILED;
    msg("em_tol= %g\n", ir->em_tol);
    // bShakeSOR
    if (!tpr_.do_bool(&bdum, data_->vergen)) return TPR_FAILED;
    // niter
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    // fc_stepsize
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    // eConstrAlg
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    // nProjOrder
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    // LincsWarnAngle
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    // nLincsIter
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    // bd_fric
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;

    if (data_->filever >= tpxv_Use64BitRandomSeed)
    {
        if (!tpr_.do_int64(&ir->ld_seed)) return TPR_FAILED;
    }
    else
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        ir->ld_seed = static_cast<int64_t>(idum);
    }
    msg("ld_seed= %lld\n", ir->ld_seed);

    INSERT_POS(press.deform); // for change deform = 
    if (!tpr_.do_vector(ir->deform, DIM*DIM, data_->prec)) return TPR_FAILED;
    print_vec("deform= ", ir->deform);

    // 余弦加速
    if (!tpr_.do_real(&ir->cos_accel, data_->prec)) return TPR_FAILED;
    msg("cos_accel= %g\n", ir->cos_accel);

    // 用户可选int和real
    if (!tpr_.do_int(&ir->userint1)) return TPR_FAILED;
    if (!tpr_.do_int(&ir->userint2)) return TPR_FAILED;
    if (!tpr_.do_int(&ir->userint3)) return TPR_FAILED;
    if (!tpr_.do_int(&ir->userint4)) return TPR_FAILED;
    if (!tpr_.do_real(&ir->userreal1, data_->prec)) return TPR_FAILED;
    if (!tpr_.do_real(&ir->userreal2, data_->prec)) return TPR_FAILED;
    if (!tpr_.do_real(&ir->userreal3, data_->prec)) return TPR_FAILED;
    if (!tpr_.do_real(&ir->userreal4, data_->prec)) return TPR_FAILED;
    msg("userint= %d %d %d %d\n", ir->userint1, ir->userint2, ir->userint3, ir->userint4);
    msg("userreal= %g %g %g %g\n", ir->userreal1, ir->userreal2, ir->userreal3, ir->userreal4);


#if 1
    // AdResS is removed, but we need to be able to read old files,
    bool bAdress = false;
    if (data_->filever >= 77 && data_->filever < tpxv_RemoveAdress)
    {
        if (!tpr_.do_bool(&bAdress, data_->vergen)) return TPR_FAILED;
        if (bAdress)
        {
            int numThermoForceGroups, numEnergyGroups;
            float rvec[DIM];
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;
            if (!tpr_.do_vector(rvec, DIM, data_->prec, data_->vergen)) return TPR_FAILED;
            if (!tpr_.do_int(&numThermoForceGroups)) return TPR_FAILED;
            if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_int(&numEnergyGroups)) return TPR_FAILED;
            if (!tpr_.do_int(&idum)) return TPR_FAILED;

            if (numThermoForceGroups > 0)
            {
                std::vector<int> idumn(numThermoForceGroups);
                if (!tpr_.do_vector(idumn.data(), numThermoForceGroups, data_->prec, data_->vergen)) return TPR_FAILED;
            }
            if (numEnergyGroups > 0)
            {
                std::vector<int> idumn(numEnergyGroups);
                if (!tpr_.do_vector(idumn.data(), numEnergyGroups, data_->prec, data_->vergen)) return TPR_FAILED;
            }
        }
    }
    msg("bAdress= %d\n", bAdress ? 1 : 0);

    // 目前温度的读取只在下面部分不存在的时候，否则就不读取温度，温度属性字节序位置设置0
    // TODO pull code
    bool bPull = false;
    if (data_->filever >= tpxv_PullCoordTypeGeom)
    {
        if (!tpr_.do_bool(&bPull, data_->vergen)) return TPR_FAILED;
    }
    else
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        bPull = (idum != 0);
    }
    if (bPull)
    {
        return TPR_SUCCESS;
        // TODO
        throw std::runtime_error("Unsupport read pull code");
    }

    // read AWH
    bool bDoAwh = false;
    if (data_->filever >= tpxv_AcceleratedWeightHistogram)
    {
        if (!tpr_.do_bool(&bDoAwh, data_->vergen)) return TPR_FAILED;
    }
    if (bDoAwh)
    {
        return TPR_SUCCESS;
        throw std::runtime_error("Unsupport read AWH code");
    }

    // Enforced rotation
    bool bRot = false;
    if (data_->filever >= 74)
    {
        if (!tpr_.do_bool(&bRot, data_->vergen)) return TPR_FAILED;
    }
    if (bRot)
    {
        return TPR_SUCCESS;
        throw std::runtime_error("Unsupport read enforced rotation code");
    }

    // IMD
    bool bIMD = false;
    if (data_->filever >= tpxv_InteractiveMolecularDynamics)
    {
        if (!tpr_.do_bool(&bIMD, data_->vergen)) return TPR_FAILED;
    }
    if (bIMD)
    {
        return TPR_SUCCESS;
        throw std::runtime_error("Unsupport read IMD code");
    }

    // 控温部分
    INSERT_POS(temperature.ngtc);
    if (!tpr_.do_int(&ir->ngtc)) return TPR_FAILED;
    msg("ir->ngtc= %d\n", ir->ngtc);
    if (data_->filever >= 69)
    {
        INSERT_POS(temperature.nhchainlength);
        if (!tpr_.do_int(&ir->nhchainlength)) return TPR_FAILED;
    }
    else
    {
        ir->nhchainlength = 1;
    }
    // 是否移除加速组功能
    if (data_->filever >= tpxv_RemovedConstantAcceleration &&
        data_->filever < tpxv_ReaddedConstantAcceleration)
    {
        ir->ngacc = 0;
    }
    else
    {
        if (!tpr_.do_int(&ir->ngacc)) return TPR_FAILED;
    }
    if (!tpr_.do_int(&ir->ngfrz)) return TPR_FAILED;
    if (!tpr_.do_int(&ir->ngener)) return TPR_FAILED;

    // allocate
    ir->nrdf.resize(ir->ngtc);
    ir->ref_t.resize(ir->ngtc);
    ir->annealing.resize(ir->ngtc);
    ir->anneal_npoints.resize(ir->ngtc);
    ir->anneal_time.resize(ir->ngtc);
    ir->anneal_temp.resize(ir->ngtc);
    ir->tau_t.resize(ir->ngtc);

    ir->nFreeze.resize(ir->ngfrz);
    ir->acceleration.resize(ir->ngacc);
    ir->egp_flags.resize(ir->ngener*ir->ngener);

    if (ir->ngtc > 0)
    { 
        if (!tpr_.do_vector(ir->nrdf.data(), ir->ngtc, data_->prec, data_->vergen)) return TPR_FAILED;

        INSERT_POS(temperature.ref_t);
        if (!tpr_.do_vector(ir->ref_t.data(), ir->ngtc, data_->prec, data_->vergen)) return TPR_FAILED;

        INSERT_POS(temperature.tau_t);
        if (!tpr_.do_vector(ir->tau_t.data(), ir->ngtc, data_->prec, data_->vergen)) return TPR_FAILED;
    }
    if (ir->ngfrz > 0)
    {
        for (int i = 0; i < ir->ngfrz; i++)
        {
            if (!tpr_.do_vector(ir->nFreeze[i].data(), DIM, data_->prec, data_->vergen)) return TPR_FAILED;
        }
    }
    if (ir->ngacc > 0)
    { 
        for (int i = 0; i < ir->ngacc; i++)
        {
            if (!tpr_.do_vector(ir->acceleration[i].data(), DIM, data_->prec, data_->vergen)) return TPR_FAILED;
        }
    }
    // egp_flags
    if (!tpr_.do_vector(ir->egp_flags.data(), ir->ngener * ir->ngener, data_->prec, data_->vergen)) return TPR_FAILED;

    // First read the lists with annealing and npoints for each group
    if (!tpr_.do_vector(ir->annealing.data(), ir->ngtc, data_->prec, data_->vergen)) return TPR_FAILED;
    if (!tpr_.do_vector(ir->anneal_npoints.data(), ir->ngtc, data_->prec, data_->vergen)) return TPR_FAILED;
    for (int i = 0; i < ir->ngtc; i++)
    {
        int k = ir->anneal_npoints[i]; // 每组点数
        ir->anneal_time[i].resize(k);
        ir->anneal_temp[i].resize(k);
        if (!tpr_.do_vector(ir->anneal_time[i].data(), k, data_->prec, data_->vergen)) return TPR_FAILED;
        if (!tpr_.do_vector(ir->anneal_temp[i].data(), k, data_->prec, data_->vergen)) return TPR_FAILED;

    }

    // 墙
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        msg("The number of wall= %d\n", idum);
        if (!tpr_.do_int(&idum)) return TPR_FAILED; // int to enum
        msg("The type of wall= %d\n", idum);
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        msg("wall_r_linpot= %g\n", rdum);
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        msg("wall_atomtype[0]= %d\n", idum);
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        msg("wall_atomtype[1]= %d\n", idum);
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        msg("wall_density[0]= %g\n", rdum);
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        msg("wall_density[1]= %g\n", rdum);
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        msg("wall_ewald_zfac= %g\n", rdum);
    }

    // 低版本gmx电场
    if (data_->filever < tpxv_GenericParamsForElectricField)
    {
        ir->elec_field.resize(12, 0.0f); // init zero
        INSERT_POS(ef);
        for (int i = 0; i < DIM; i++)
        {
            // n和nt不可大于1
            int n, nt;
            if (!tpr_.do_int(&n)) return TPR_FAILED;
            if (!tpr_.do_int(&nt)) return TPR_FAILED;
            ir->elec_old_gmx[i].n = n;
            ir->elec_old_gmx[i].nt = nt;

            // +1 确保容器正常
            std::vector<float> aa(n + 1), phi(nt + 1), at(nt + 1), phit(nt + 1);
            if (!tpr_.do_vector(aa.data(),  n, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_vector(phi.data(), n, data_->prec)) return TPR_FAILED;
            if (!tpr_.do_vector(at.data(),  nt,data_->prec)) return TPR_FAILED;
            if (!tpr_.do_vector(phit.data(),nt,data_->prec)) return TPR_FAILED;
            msg("n= %d, nt= %d\n", n, nt);
            if (n > 0)
            {
                if (n > 1 || nt > 1)
                {
                    throw std::runtime_error("Can not handle tpr files with more than one electric field term per direction.");
                }
                msg("dim=%d, E0=    %g\n", i, aa[0]);
                msg("dim=%d, omega= %g\n", i, at[0]);
                msg("dim=%d, t0=    %g\n", i, phi[0]);
                msg("dim=%d, sigma= %g\n", i, phit[0]);

                // store ef
                ir->elec_field[i * 4 + 0] = aa[0];
                ir->elec_field[i * 4 + 1] = at[0];
                ir->elec_field[i * 4 + 2] = phi[0];
                ir->elec_field[i * 4 + 3] = phit[0];
            }
        }
        print_vec("ElecX= ", ir->elec_field.data() + 0, 4);
        print_vec("ElecY= ", ir->elec_field.data() + 4, 4);
        print_vec("ElecZ= ", ir->elec_field.data() + 8, 4);
    }

    // 计算电生理学: 未完成
    if (data_->filever >= tpxv_ComputationalElectrophysiology)
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED; // int to enum
        msg("swapcoords= %d\n", idum); // No=0, X, Y, Z
        if (idum != 0)
        {
            // do_swapcoords_tpx 
            return TPR_SUCCESS;
            throw std::runtime_error("Unsupport Computational Electrophysiology");
        }
    }

    // QMMM
    bool bQMMM = false;
    if (!tpr_.do_bool(&bQMMM, data_->vergen)) return TPR_FAILED;
    msg("bQMMM= %d\n", bQMMM ? 1 : 0);
    if (!tpr_.do_int(&idum)) return TPR_FAILED; // qmmmScheme
    msg("qmmmScheme= %d\n", idum);
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED; // unusedScalefactor
    msg("unusedScalefactor= %g\n", rdum);
    int ngQM = 0;
    if (!tpr_.do_int(&ngQM)) return TPR_FAILED;
    msg("ngQM= %d\n", ngQM);
    // 读旧的QMMM参数
    if (bQMMM && ngQM > 0)
    {
        std::vector<int> dumi(4 * ngQM, 0);
        if (!tpr_.do_vector(dumi.data(), (int)dumi.size(), data_->prec)) return TPR_FAILED;

        // std::vector<bool> has no data()
        std::vector<char> dumc(ngQM * sizeof(bool) / sizeof(char));
        if (!tpr_.do_vector(reinterpret_cast<bool*>(dumc.data()), (int)dumc.size(), data_->prec)) return TPR_FAILED;

        dumi.resize(2 * ngQM, 0);
        if (!tpr_.do_vector(dumi.data(), (int)dumi.size(), data_->prec)) return TPR_FAILED;

        std::vector<float> dumf(2 *ngQM, 0);
        if (!tpr_.do_vector(dumf.data(), (int)dumf.size(), data_->prec)) return TPR_FAILED;

        dumi.resize(3 * ngQM, 0);
        if (!tpr_.do_vector(dumi.data(), (int)dumi.size(), data_->prec)) return TPR_FAILED;
    }

    // 高版本gmx电场：此处为树结构
    if (data_->filever >= tpxv_GenericParamsForElectricField)
    {
        // 目前只针对applied-forces中含有电场部分，如果有其他部分，则读取失败，直接返回成功状态，因为我不想让tpr读取功能崩溃
        ir->elec_field.resize(12, 0.0f);
        INSERT_POS(ef);
        try
        {
            char tempstr[MAX_LEN];
            unsigned char typeTag;

            if (!tpr_.do_int(&data_->ir.elec_nf)) return TPR_FAILED;
            msg("nf count= %d\n", data_->ir.elec_nf);
            // 'applied-forces' item
            for (int i = 0; i < data_->ir.elec_nf; i++)
            {
                if (!tpr_.do_string(tempstr, data_->vergen)) return TPR_FAILED;
                msg("name= '%s'\n", tempstr);
                if (!tpr_.do_uchar(&typeTag, data_->vergen)) return TPR_FAILED;
                msg("typeTag= '%c'\n", typeTag); // 'O' -> obj

                // 'electric-field' when j==0
                if (!tpr_.do_int(&data_->ir.elec_ne)) return TPR_FAILED;
                msg("ne count= %d\n", data_->ir.elec_ne);
                for (int j = 0; j < data_->ir.elec_ne; j++)
                {
                    if (!tpr_.do_string(tempstr, data_->vergen)) return TPR_FAILED;
                    msg("name= '%s'\n", tempstr);
                    if (!tpr_.do_uchar(&typeTag, data_->vergen)) return TPR_FAILED;
                    msg("typeTag= '%c'\n", typeTag); // 'O' -> obj

                    // x or y or z
                    int ndim;
                    if (!tpr_.do_int(&ndim)) return TPR_FAILED;
                    msg("nx count= %d\n", ndim); // == DIM
                    for (int k = 0; k < ndim; k++)
                    {
                        if (!tpr_.do_string(tempstr, data_->vergen)) return TPR_FAILED;
                        msg("name= '%s'\n", tempstr);
                        if (!tpr_.do_uchar(&typeTag, data_->vergen)) return TPR_FAILED;
                        msg("typeTag= '%c'\n", typeTag); // 'O' -> obj

                        // E0, omega, t0, sigma
                        int nd;
                        if (!tpr_.do_int(&nd)) return TPR_FAILED;
                        msg("nd count= %d\n", nd); // == 4
                        myassert(nd == 4, "The electricfield size must have four parameters");

                        for (int m = 0; m < nd; m++)
                        {
                            if (!tpr_.do_string(tempstr, data_->vergen)) return TPR_FAILED;
                            msg("name= '%s'\n", tempstr);
                            if (!tpr_.do_uchar(&typeTag, data_->vergen)) return TPR_FAILED;
                            msg("typeTag= '%c'\n", typeTag); // 'f' -> float

                            if (!tpr_.do_real(&ir->elec_field[k * nd + m], data_->prec)) return TPR_FAILED;
                        }
                    }

                    // TODO: when ir.elec_ne>1 for high gromacs , support 'density-guided-simulation', 'qmmm-cp2k:' to read
                    break;
                }
            }
        }
        catch (const std::exception&e)
        {
            msg("Warning! Electric field paramaters can not be read: %s\n", e.what());
            return TPR_SUCCESS;
        }
        print_vec("ElecX= ", ir->elec_field.data()+0, 4);
        print_vec("ElecY= ", ir->elec_field.data()+4, 4);
        print_vec("ElecZ= ", ir->elec_field.data()+8, 4);
    }

#endif // 0

    return TPR_SUCCESS;
}

bool TprReader::do_fepvals()
{
    bool        bdum;
    float       rdum;
    int         idum;
    double      d;

    if (data_->filever >= 79)
    {
        //init_fep_state, init_lambda,delta_lambda
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        if (!tpr_.do_double(&d)) return TPR_FAILED;
        if (!tpr_.do_double(&d)) return TPR_FAILED;
    }
    else if (data_->filever >= 59)
    {
        if (!tpr_.do_double(&d)) return TPR_FAILED;
        if (!tpr_.do_double(&d)) return TPR_FAILED;
    }
    else
    {
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    }

    if (data_->filever >= 79)
    {
        //n_lambda
        if (!tpr_.do_int(&data_->ir.n_lambda)) return TPR_FAILED;
        // 遍历Fep, Mass, Coul, Vdw, Bonded, Restraint, Temperature
        const int Count = 7;
        for (int i = 0; i < Count; i++)
        {
            if (data_->ir.n_lambda > 0)
            {
                std::vector<double> lambda(data_->ir.n_lambda);
                if (!tpr_.do_vector(lambda.data(), data_->ir.n_lambda)) return TPR_FAILED;
                bool   separate_dvdl;
                for (int j = 0; j < Count; j++)
                {
                    if (!tpr_.do_bool(&separate_dvdl, data_->vergen)) return TPR_FAILED;
                }
            }
        }
    }
    else if (data_->filever >= 64)
    {
        //n_lambda
        if (!tpr_.do_int(&data_->ir.n_lambda)) return TPR_FAILED;

        std::vector<double> lambda(data_->ir.n_lambda);
        if (!tpr_.do_vector(lambda.data(), data_->ir.n_lambda)) return TPR_FAILED;
    }
    else
    {
        data_->ir.n_lambda = 0;
    }
    msg("n_lambda= %d\n", data_->ir.n_lambda);

    // sc_alpha
    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    msg("sc_alpha= %g\n", rdum);
    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    msg("sc_power= %d\n", idum);
    if (data_->filever >= 79)
    {
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    }
    else
    {
        rdum = 6.0f;
    }
    msg("sc_r_power= %g\n", rdum);

    if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    msg("sc_sigma= %g\n", rdum);

    if (data_->filever >= 79)
    {
        if (!tpr_.do_bool(&bdum, data_->vergen)) return TPR_FAILED;
    }
    else
    {
        idum = true;
    }
    msg("bScCoul= %d\n", idum ? 1 : 0);

    //nstdhdl
    if (data_->filever >= 64)
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
    }
    else
    {
        idum = 1;
    }
    msg("nstdhdl= %d\n", idum);

    if (data_->filever >= 73)
    {
        // enum as int, separate_dhdl_file, dhdl_derivatives
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
    }
    if (data_->filever >= 71)
    {
        // dh_hist_size
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        // dh_hist_spacing
        if (!tpr_.do_double(&d)) return TPR_FAILED;
    }
    if (data_->filever >= 79)
    {
        // enum as int, edHdLPrintEnergy
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
    }
    if (data_->filever >= tpxv_SoftcoreGapsys)
    {
        // softcoreFunction, scGapsysScaleLinpointLJ, scGapsysScaleLinpointQ, scGapsysSigmaLJ
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
        if (!tpr_.do_real(&rdum, data_->prec)) return TPR_FAILED;
    }
    
    // lambda_neighbors
    if ((data_->filever >= 83 && data_->filever < 90) || data_->filever >= 92)
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
    }

    return TPR_SUCCESS;
}

bool TprReader::do_groups()
{
    //do_grps
    int                 idum, nr;
    vecI2D              gid(egcNR); // 每个类型中的原子组编号
    std::vector<int>    gpos; //每个原子组字符串的起始位置
    for (int i = 0; i < egcNR; i++)
    {
        // i=0时，温度耦合组数目 grp[T-Coupling  ] 
        if (!tpr_.do_int(&nr)) return TPR_FAILED;

        gid[i].resize(nr); // allocated memory for this type
        if (!tpr_.do_vector(gid[i].data(), nr, data_->prec)) return TPR_FAILED;
    }

    if (!tpr_.do_int(&idum)) return TPR_FAILED;
    msg("number of group names= %d\n", idum);
    // 每个原子组字符串的起始位置
    gpos.resize(idum);
    for (int i = 0; i < idum; i++)
    {
        if (!tpr_.do_int(&gpos[i])) return TPR_FAILED;
    }

    for (int i = 0; i < egcNR; i++)
    {
        if (!tpr_.do_int(&idum)) return TPR_FAILED;
        if (idum != 0)
        {
            // for gmx >= 2020，uchar只读1字节
            std::vector<unsigned char> temp(idum);
            tpr_.do_vector(temp.data(), idum, data_->prec, data_->vergen);
        }
    }

    // 输出每种类型组数目和组名称
    for (int i = 0; i < egcNR; i++)
    {
        msg("grp[%-12s] nr= %zu, name= [", c_groups[i], gid[i].size());
        for (const auto& id : gid[i])
        {
            const char* gpname = &data_->symtab[SAVELEN * gpos[id]];
#ifdef _DEBUG
            fprintf(stderr, " %s", gpname);
        }
        fprintf(stderr, "]\n");
#else
        }
#endif // _DEBUG
    }

    return TPR_SUCCESS;
}

bool TprReader::do_ilists(int ntype, std::vector<int> (&nr)[F_NRE], vecI2D (&interactionlist)[F_NRE])
{
    for (int i = 0; i < F_NRE; i++)
    {
        bool bClear = false;
        for (int k = 0; k < NFTUPD; k++)
        {
            if ((data_->filever < ftupd[k].fvnr) && (i == ftupd[k].ftype))
            {
                bClear = true;
            }
        }

        if (bClear)
        {
            nr[i][ntype] = 0;
            interactionlist[i][ntype].clear();
        }
        else
        {
            // get the number of interactions
            if (!tpr_.do_int(&nr[i][ntype])) return TPR_FAILED;
            if (nr[i][ntype] == 0) continue; // empty

            // allocated memory
            interactionlist[i][ntype].resize(nr[i][ntype]);
            if (!tpr_.do_vector(interactionlist[i][ntype].data(), nr[i][ntype], data_->prec)) return TPR_FAILED;

            if constexpr (0)
            {
                msg("%d ", nr[i][ntype]);
                for (int j = 0; j < nr[i][ntype]; j++)
                {
                    printf("%d ", interactionlist[i][ntype][j]);
                }
                printf("\n");
            }
        }
    }

    return TPR_SUCCESS;
}


bool TprReader::set_nsteps(int64_t nsteps)
{
    long            fsize = 0;
    const char* buffer = tpr_.get_file_buffer(&fsize);
    // 原始位置不为0
    if (fsize && data_->property.nsteps)
    {
        FileSerializer  newtpr(fout_, "wb");

        // write nsteps before 
        if (newtpr.fwrite_(buffer, data_->property.nsteps * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in set_nsteps before");
        }

        // write new nsteps
        if (data_->filever >= 62)
        {
            if (!newtpr.do_int64(&nsteps)) return TPR_FAILED;
        }
        else
        {
            // old tpr use int type
            int idum = static_cast<int>(nsteps);
            if (!newtpr.do_int(&idum)) return TPR_FAILED;
        }

        // write nsteps after
        long len = fsize - data_->property.nsteps - sizeof(int64_t);
        if (newtpr.fwrite_(&buffer[data_->property.nsteps + sizeof(int64_t)], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in set_nsteps after");
        }

        return TPR_SUCCESS;
    }
    return TPR_FAILED;
}

bool TprReader::set_dt(double dt)
{
    long            fsize = 0;
    int             realsize = 8;
    const char* buffer = tpr_.get_file_buffer(&fsize);
    // 原始位置不为0
    if (fsize && data_->property.dt)
    {
        FileSerializer  newtpr(fout_, "wb");

        // write dt before 
        if (newtpr.fwrite_(buffer, data_->property.dt * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in set_dt before");
        }

        // write new dt 
        if (data_->filever >= 59)
        {
            if (!newtpr.do_double(&dt)) return TPR_FAILED;
            realsize = 8;
        }
        else
        {
            // old tpr use real type
            realsize = data_->prec;
            float rdum = static_cast<float>(dt);
            if (!newtpr.do_real(&rdum, data_->prec)) return TPR_FAILED;
        }

        // write dt after
        long len = fsize - data_->property.dt - realsize;
        if (newtpr.fwrite_(&buffer[data_->property.dt + realsize], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in set_dt after");
        }

        return TPR_SUCCESS;
    }
    return TPR_FAILED;
}


//< set up relative box
// \param[in] deform: the ir->deform 
// \param[out] box_rel: the relative box 
// \param[in/out] b: the system box
// \param[in] bInit: get box_rel if true, else return b
static inline void do_box_rel(
             int ndim,
             const float deform[DIM][DIM], 
             float box_rel[DIM][DIM], 
             float b[DIM][DIM],
             bool bInit
)
{
    for (int d = YY; d <= ZZ; ++d)
    {
        for (int d2 = XX; d2 < ndim; ++d2)
        {
            /* We need to check if this box component is deformed
             * or if deformation of another component might cause
             * changes in this component due to box corrections.
             */
            if (deform[d][d2] == 0
                && !(d == ZZ && d2 == XX && deform[d][YY] != 0 && (b[YY][d2] != 0 || deform[YY][d2] != 0)))
            {
                if (bInit)
                {
                    box_rel[d][d2] = b[d][d2] / b[XX][XX];
                }
                else
                {
                    b[d][d2] = b[XX][XX] * box_rel[d][d2];
                }
            }
        }
    }
}



bool TprReader::set_pressure(
    const char* method, const char* type, float tau_p, 
    std::vector<float>& ref_p, 
    std::vector<float>& compress,
    std::vector<float>& deform)
{
    // check data type float must be same as data_->prec
    if (sizeof(float) != data_->prec)
    {
        throw std::runtime_error("set_pressure only support single precision tpr");
    }

    // check pressure coupling keywords
    PressureCoupling epc;
    PressureCouplingType epct;
    if ((epc = check_string<PressureCoupling>(method, c_PressureCoupling)) == PressureCoupling::Count)
    {
        throw std::runtime_error(std::string("Unknown pressure coupling method: ") + method);
    }
    if ((epct = check_string<PressureCouplingType>(type, c_PressureCouplingType)) == PressureCouplingType::Count)
    {
        throw std::runtime_error(std::string("Unknown pressure coupling type: ") + type);
    }
    // ref_p and compress
    if (ref_p.size() != DIM * DIM)
    {
        throw std::runtime_error("The size of ref_p must be 9");
    }
    if (compress.size() != DIM * DIM)
    {
        throw std::runtime_error("The size of compressibility must be 9");
    }
    // deform
    if (deform.size() != DIM * DIM)
    {
        throw std::runtime_error("The size of deform must be 9");
    }

    // only for fileversion >= 51
    if (data_->filever < 51)
    {
        throw std::runtime_error("Only support tpr file version >= 51 to write");
    }

    long            fsize = 0;
    const char* buffer = tpr_.get_file_buffer(&fsize);
    if (fsize && !data_->property.press.empty())
    {
        // set pressure parameters
        FileSerializer  newtpr(fout_, "wb");

        // write box_rel before 
        if (newtpr.fwrite_(buffer, data_->property.press.box_rel * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in box_rel before");
        }

        // write box_rel
        float box_rel[DIM * DIM] = { 0 };
        if (epc != PressureCoupling::No &&
            data_->ir.deform[XX] == 0  &&
            (epct == PressureCouplingType::Isotropic || epct == PressureCouplingType::SemiIsotropic))
        {
            const int ndim = (epct == PressureCouplingType::SemiIsotropic) ? 2 : 3;
            do_box_rel(ndim,
                (const float(*)[DIM])data_->ir.deform,
                (float(*)[DIM])box_rel,
                (float(*)[DIM])data_->box.data(),
                true);
        }
        if (!newtpr.do_vector(box_rel, DIM * DIM, data_->prec, data_->vergen)) return TPR_FAILED;


        // write box_rel after and epc before
        constexpr size_t box_relsize = DIM * DIM * sizeof(float);
        long len = data_->property.press.epc - data_->property.press.box_rel - box_relsize;
        if (newtpr.fwrite_(&buffer[data_->property.press.box_rel + box_relsize], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in set_pressure before");
        }

        // write epc as int enum
        int enum_epc = static_cast<int>(epc);
        if (!newtpr.do_int(&enum_epc)) return TPR_FAILED;

        // write epct as int enum
        int enum_epct = static_cast<int>(epct);
        if (!newtpr.do_int(&enum_epct)) return TPR_FAILED;

        // write old nstpcouple
        if (data_->filever >= 71)
        {
            if (!newtpr.do_int(&data_->ir.nstpcouple)) return TPR_FAILED;
        }

        // write tau_p in float
        if (!newtpr.do_real(&tau_p, data_->prec)) return TPR_FAILED;

        // write ref_p in vector
        if (!newtpr.do_vector(ref_p.data(), DIM * DIM, data_->prec, data_->vergen)) return TPR_FAILED;

        // write compress in vector
        if (!newtpr.do_vector(compress.data(), DIM * DIM, data_->prec, data_->vergen)) return TPR_FAILED;

        // write compress after and deform before
        constexpr size_t size = sizeof(float) * DIM * DIM;
        len = data_->property.press.deform - data_->property.press.compress - size;
        if (newtpr.fwrite_(&buffer[data_->property.press.compress + size], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in set_pressure after and deform before");
        }

        // write deform in vector
        if (!newtpr.do_vector(deform.data(), DIM * DIM, data_->prec, data_->vergen)) return TPR_FAILED;

        // write deform after
        len = fsize - data_->property.press.deform - size;
        if (newtpr.fwrite_(&buffer[data_->property.press.deform + size], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in set_pressure deform after");
        }

        return TPR_SUCCESS;
    }

    return TPR_FAILED;
}

bool TprReader::set_temperature(
    const char* method,
    std::vector<float>& tau_t,
    std::vector<float>& ref_t
)
{
    // check data type float must be same as data_->prec
    if (sizeof(float) != data_->prec)
    {
        throw std::runtime_error("set_temperature only support single precision tpr");
    }

    // check temperature coupling keywords
    TemperatureCoupling etc;
    if ((etc = check_string<TemperatureCoupling>(method, c_TemperatureCoupling)) == TemperatureCoupling::Count)
    {
        throw std::runtime_error(std::string("Unknown temperature coupling method: ") + method);
    }
    // ref_p and compress
    if (ref_t.size() != tau_t.size())
    {
        throw std::runtime_error("The size of ref_t and tau_t must be same");
    }
    if (static_cast<int>(ref_t.size()) != data_->ir.ngtc)
    {
        throw std::runtime_error(std::string("The size of ref_t must be same as old tpr: ") + std::to_string(data_->ir.ngtc));
    }

    long            fsize = 0;
    const char* buffer = tpr_.get_file_buffer(&fsize);
    if (fsize && !data_->property.temperature.empty())
    {
        // set_temperature parameters
        FileSerializer  newtpr(fout_, "wb");

        // write etc before 
        if (newtpr.fwrite_(buffer, data_->property.temperature.etc * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in ir->etc before");
        }

        // write etc type as enum
        int enum_etc = static_cast<int>(etc);
        if (!newtpr.do_int(&enum_etc)) return TPR_FAILED;

        // write etc after and ngtc before
        constexpr int sizeInt = (int)sizeof(int); // etc size
        long len = data_->property.temperature.ngtc - data_->property.temperature.etc - sizeInt;
        if (newtpr.fwrite_(&buffer[data_->property.temperature.etc + sizeInt], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in etc after and ir->ngtc before");
        }

        // write ir->ngtc 
        if (!newtpr.do_int(&data_->ir.ngtc)) return TPR_FAILED;

        // write nhchainlength
        if (data_->filever >= 69)
        {
            int nhchainlength = data_->ir.nhchainlength;
            if (etc == TemperatureCoupling::NoseHoover)
            {
                nhchainlength = 1; // use default value 1
            }
            if (!newtpr.do_int(&nhchainlength)) return TPR_FAILED;
        }

        // write nhchainlength after and ref_t before
        // if has nhchainlength position
        long started = data_->filever >= 69 ? data_->property.temperature.nhchainlength : data_->property.temperature.ngtc;
        len = data_->property.temperature.ref_t - started - sizeInt;
        if (newtpr.fwrite_(&buffer[started + sizeInt], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in nhchainlength after and ref_t before");
        }

        // write ref_t vector
        if (!newtpr.do_vector(ref_t.data(), data_->ir.ngtc, data_->prec, data_->vergen)) return TPR_FAILED;

        // write tau_t vector
        if (!newtpr.do_vector(tau_t.data(), data_->ir.ngtc, data_->prec, data_->vergen)) return TPR_FAILED;

        // write tau_t after
        const int size = (int)sizeof(float) * data_->ir.ngtc;
        len = fsize - data_->property.temperature.tau_t - size;
        if (newtpr.fwrite_(&buffer[data_->property.temperature.tau_t + size], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in ir->tau_t after");
        }

        return TPR_SUCCESS;
    }

    return TPR_FAILED;
}

bool TprReader::set_mdp_integer(const char* prop, int val)
{
	ParamsInteger epi;
	if ((epi = check_string<ParamsInteger>(prop, c_mdp_integer)) == ParamsInteger::Count)
	{
        throw std::runtime_error(std::string("Unknown mdp property: ") + prop);
	}

    // too old tpr unsupport
    if (data_->filever < 71)
    {
        throw std::runtime_error(std::string("Too old tpr file version: ") + std::to_string(data_->filever));
    }

    long            fsize = 0;
    const char* buffer = tpr_.get_file_buffer(&fsize);
    if (fsize && !data_->property.integer.empty())
    {
        // set_mdp_integer parameters
        FileSerializer  newtpr(fout_, "wb");

        // write epi before 
        int eIdx = static_cast<int>(epi);
        const long* pos = &data_->property.integer.nstlog; // a pointer to struct start pos
        long keypos = *(pos + eIdx);
        if (newtpr.fwrite_(buffer, keypos * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in keyword before");
        }

        // write new epi in a int
        if (!newtpr.do_int(&val)) return TPR_FAILED;

        // write keyword after
        constexpr int size = sizeof(int);
        long len = fsize - keypos - size;
        if (newtpr.fwrite_(&buffer[keypos + size], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in keyword after");
        }

        return TPR_SUCCESS;
    }

    return TPR_FAILED;
}

const std::vector<float>& TprReader::get_xvf(const char* type) const
{
    // check input type, must X, or V or F or M or Q or box or electric field
    VecProps evec;
    if ((evec = check_string<VecProps>(type, c_mdp_vector)) == VecProps::Count)
    {
        throw std::runtime_error(std::string("Unknown vector property: ") + type);
    }

    switch (evec)
    {
    case VecProps::x:
    {
        // check if has coordinates of tpr
        if (!data_->bX)
        {
            throw std::runtime_error("Input tpr has not coordinates information");
        }
        return data_->atoms.x;
    }
    case VecProps::v:
    {
        // check if has velocity of tpr
        if (!data_->bV)
        {
            throw std::runtime_error("Input tpr has not velocity information");
        }
        return data_->atoms.v;
    }
    case VecProps::f:
    {
        // check if has force of tpr
        if (!data_->bF)
        {
            throw std::runtime_error("Input tpr has not force information");
        }
        return data_->atoms.f;
    }
    case VecProps::m:
    {
        // check if has get mass
        if (data_->atoms.mass.empty())
        {
            throw std::runtime_error("Can not get mass information");
        }
        return data_->atoms.mass;
    }
    case VecProps::q:
    {
        // check if has get charge
        if (data_->atoms.charge.empty())
        {
            throw std::runtime_error("Can not get charge information");
        }
        return data_->atoms.charge;
    }
    case VecProps::box:
    {
        // check if has get charge
        if (!data_->bBox)
        {
            throw std::runtime_error("Have not box information in tpr");
        }
        return data_->box;
    }
    case VecProps::ef:
    {
        return get_ef();
    }
    default:
        throw std::invalid_argument(std::string("Unknown keyword: ") + type);
        break;
    }
}

const std::vector<int>& TprReader::get_ivector(const char* type) const
{
    IVectorProps evec;
    if ((evec = check_string<IVectorProps>(type, c_int_vector)) == IVectorProps::Count)
    {
        throw std::runtime_error(std::string("Unknown int vector property: ") + type);
    }

    switch (evec)
    {
    case IVectorProps::resid:
    {
        if (data_->atoms.resid.empty())
        {
            throw std::runtime_error("Can not get resid information");
        }
        return data_->atoms.resid;
    }
    case IVectorProps::atnum:
    {
        if (data_->atoms.atomtypenumber.empty())
        {
            throw std::runtime_error("Can not get atomtype number information");
        }
        return data_->atoms.atomtypenumber;
    }
    case IVectorProps::atomicnum:
    {
        if (data_->atoms.atomnumber.empty() || 
            std::all_of(data_->atoms.atomnumber.begin(), data_->atoms.atomnumber.end(),
                [](int val) {return val == -1; })
            )
        {
            throw std::runtime_error("Can not get atomic number information or all -1");
        }
        return data_->atoms.atomnumber;
    }
    default:
        throw std::invalid_argument(std::string("Unknown keyword: ") + type);
        break;
    }
}

const std::vector<std::string>& TprReader::get_name(const char* type) const
{
    // check input, res, atom name
    StringType evec;
    if ((evec = check_string<StringType>(type, c_name_vector)) == StringType::Count)
    {
        throw std::runtime_error(std::string("Unknown vector property: ") + type);
    }

    switch (evec)
    {
    case StringType::res:
    {
        if (data_->atoms.resname.empty())
        {
            throw std::runtime_error("Can not get resname information");
        }
        return data_->atoms.resname;
    }
    case StringType::atom:
    {
        if (data_->atoms.atomname.empty())
        {
            throw std::runtime_error("Can not get atomname information");
        }
        return data_->atoms.atomname;
    }
    case StringType::type:
    {
        if (data_->atoms.atomtypename.empty())
        {
            throw std::runtime_error("Can not get atomtypename information");
        }
        return data_->atoms.atomtypename;
    }
    default:
        throw std::invalid_argument(std::string("Unknown keyword: ") + type);
        break;
    }
}

const std::vector<Bonded> &TprReader::get_bonded(const char *type) const
{
    // check input
    BondedType evec;
    if ((evec = check_string<BondedType>(type, c_bonded_type)) == BondedType::Count)
    {
        throw std::runtime_error(std::string("Unknown bonded property: ") + type);
    }
    
    switch (evec)
    {
    case BondedType::bonds:
    {
        if (data_->bonds.empty())
        {
            throw std::runtime_error("Can not get bonds information from tpr");
        }
        return data_->bonds; 
    }
    case BondedType::angles:
    {
        if (data_->angles.empty())
        {
            throw std::runtime_error("Can not get angles information from tpr");
        }
        return data_->angles; 
    }
    case BondedType::dihedrals:
    {
        if (data_->dihedrals.empty())
        {
            throw std::runtime_error("Can not get dihedrals information from tpr");
        }
        return data_->dihedrals;
    }
    case BondedType::impropers:
    {
        if (data_->dihedrals.empty())
        {
            throw std::runtime_error("Can not get dihedrals information from tpr");
        }
        return data_->impropers;
    }
    default:
        throw std::invalid_argument(std::string("Unknown keyword: ") + type);
        break;
    }
}

const std::vector<NonBonded>& TprReader::get_nonbonded(const char* type) const
{
    // check input
    NonBondedType evec;
    if ((evec = check_string<NonBondedType>(type, c_nonbonded_type)) == NonBondedType::Count)
    {
        throw std::runtime_error(std::string("Unknown nonbonded property: ") + type);
    }

    switch (evec)
    {
    case NonBondedType::LJ:
        if (data_->ljparams.empty())
        {
            throw std::runtime_error("Can not get LJ information from tpr");
        }
        return data_->ljparams;
    case NonBondedType::atomtype:
        if (data_->atomtypesLJ.empty())
        {
            throw std::runtime_error("Can not get atomtype LJ information from tpr");
        }
        return data_->atomtypesLJ;
    case NonBondedType::LJ_14:
        if (data_->pairs.empty())
        {
            throw std::runtime_error("Can not get LJ_14(pairs) information from tpr");
        }
        return data_->pairs;
    default:
        throw std::invalid_argument(std::string("Unknown keyword: ") + type);
        break;
    }
}


int TprReader::get_mdp_integer(const char* prop) const
{
    ParamsInteger epi;
    if ((epi = check_string<ParamsInteger>(prop, c_mdp_integer)) == ParamsInteger::Count)
    {
        throw std::runtime_error(std::string("Unknown mdp property: ") + prop);
    }

    switch (epi)
    {
    case ParamsInteger::nstlog:
        return data_->ir.nstlog;
    case ParamsInteger::nstxout:
        return data_->ir.nstxout;
    case ParamsInteger::nstvout:
        return data_->ir.nstvout;
    case ParamsInteger::nstfout:
        return data_->ir.nstfout;
    case ParamsInteger::nstenergy:
        return data_->ir.nstenergy;
    case ParamsInteger::nstxout_compressed:
        return data_->ir.nstxout_compressed;
    case ParamsInteger::nsttcouple:
        return data_->ir.nsttcouple;
    case ParamsInteger::nstpcouple:
        return data_->ir.nstpcouple;
    case ParamsInteger::nstcalcenergy:
        return data_->ir.nstcalcenergy;
    default:
        break;
    }
    return -1;
}

const std::vector<float> &TprReader::get_ef() const
{
    // 场强E0全0
    if (data_->ir.elec_field.empty() || 
        (data_->ir.elec_field[0] == 0.0f &&
         data_->ir.elec_field[4] == 0.0f &&
         data_->ir.elec_field[8] == 0.0f) )
    {
        throw std::runtime_error("Error! Have not electric field in tpr");
    }
    return data_->ir.elec_field;
}

bool TprReader::write_xvf(std::vector<float> &vec, long pos, long prec) const
{
    long            fsize = 0;
    const char* buffer = tpr_.get_file_buffer(&fsize);
    // 原始位置不为0
    if (fsize && pos)
    {
        FileSerializer  newtpr(fout_, "wb");

        // write nsteps before 
        if (newtpr.fwrite_(buffer, pos * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in write_xvf before");
        }

        // write new vector
        if (!newtpr.do_vector(vec.data(), (int)vec.size(), prec)) return TPR_FAILED;

        // write vector after
        size_t size = vec.size() * sizeof(float);
        long len = fsize - pos - (long)size;
        if (newtpr.fwrite_(&buffer[pos + size], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in write_xvf after");
        }
        return TPR_SUCCESS;
    }

    return TPR_FAILED;
}

bool TprReader::write_ef(std::vector<float>& vec, long pos, long prec) const
{
    long            fsize = 0;
    const char* buffer = tpr_.get_file_buffer(&fsize);

    if (!(fsize && pos)) return TPR_FAILED;

    // 低版本电场
    if (data_->filever < tpxv_GenericParamsForElectricField)
    {
        FileSerializer  newtpr(fout_, "wb");
        // write ef before 
        if (newtpr.fwrite_(buffer, pos * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in write_ef before");
        }

        auto tempvec(vec); // copy 
        // ajdust order, vec：E0, omega, t0, sigma
        for (int i = 0; i < DIM; i++) {
            std::swap(tempvec[i * 4 + 1], tempvec[i * 4 + 2]);
        }
        long nskip = 0; // how many bytes to write for electric field
        for (int i = 0; i < DIM; i++)
        {
            // write n, nt
            int n = data_->ir.elec_old_gmx[i].n;
            int nt = data_->ir.elec_old_gmx[i].nt;
            if (!newtpr.do_int(&n))  return TPR_FAILED;
            if (!newtpr.do_int(&nt)) return TPR_FAILED;
            nskip += (n + nt) * (2 * sizeof(float) + sizeof(int));

            // write E0, t0, omega, sigma in DIM of tpr, n or nt may be zero
            if (!newtpr.do_vector(tempvec.data() + i * 4 + 0, n, prec)) return TPR_FAILED;
            if (!newtpr.do_vector(tempvec.data() + i * 4 + 1, n, prec)) return TPR_FAILED;
            if (!newtpr.do_vector(tempvec.data() + i * 4 + 2, nt, prec)) return TPR_FAILED;
            if (!newtpr.do_vector(tempvec.data() + i * 4 + 3, nt, prec)) return TPR_FAILED;
        }
        
        // write ef after 
        long len = fsize - pos - nskip;
        if (newtpr.fwrite_(&buffer[pos + nskip], len * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in write_ef after");
        }

        return TPR_SUCCESS;
    }
    // 高版本电场
    else if (data_->filever >= tpxv_GenericParamsForElectricField &&
            data_->ir.elec_nf == 1 && data_->ir.elec_ne >= 1)
    {
        FileSerializer  newtpr(fout_, "wb");
        // write ef before 
        if (newtpr.fwrite_(buffer, pos * sizeof(char), 1) != 1)
        {
            throw std::runtime_error("fwrite_ error in write_ef before");
        }

        //! write electric field
        // 1. Firstly write nf,  'applied-forces' string and type
        if (!newtpr.do_int(&data_->ir.elec_nf)) return TPR_FAILED;
        char str[] = "applied-forces";
        unsigned char otype = 'O'; // obj
        if (!newtpr.do_string(str, data_->vergen)) return TPR_FAILED;
        if (!newtpr.do_uchar(&otype, data_->vergen)) return TPR_FAILED;

        // 2. Then write ne and 'electric-field' and type
        if (!newtpr.do_int(&data_->ir.elec_ne)) return TPR_FAILED;
        char str2[] = "electric-field";
        if (!newtpr.do_string(str2, data_->vergen)) return TPR_FAILED;
        if (!newtpr.do_uchar(&otype, data_->vergen)) return TPR_FAILED;

        // 3. write ndim and electric field value in three dimension
        int ndim = DIM;
        if (!newtpr.do_int(&ndim)) return TPR_FAILED;
        const char* direct[] = { "x", "y", "z" };
        const char* items[] = { "E0", "omega", "t0", "sigma"};
        unsigned char ftype = 'f'; // float
        for (int i = 0; i < DIM; i++)
        {
            if (!newtpr.do_string((char *)direct[i], data_->vergen)) return TPR_FAILED;
            if (!newtpr.do_uchar(&otype, data_->vergen)) return TPR_FAILED;

            int nd = 4; // E0, Omega, t0, sigma
            if (!newtpr.do_int(&nd)) return TPR_FAILED;
            for (int j = 0; j < nd; j++)
            {
                if (!newtpr.do_string((char*)items[j], data_->vergen)) return TPR_FAILED;
                if (!newtpr.do_uchar(&ftype, data_->vergen)) return TPR_FAILED;
                // 电场实际值
                if (!newtpr.do_real(&vec[i * nd + j], data_->prec)) return TPR_FAILED;
            }
        }

        // write ef after
        long currpos = static_cast<long>(newtpr.ftell_()); // 理论上应该当前位置就处于文件结尾了
        if (currpos < fsize)
        {
            if (newtpr.fwrite_(&buffer[currpos], (fsize - currpos) * sizeof(char), 1) != 1)
            {
                throw std::runtime_error("fwrite_ error in write_ef after");
            }
        }

        return TPR_SUCCESS;
    }

    return TPR_FAILED;
}

bool TprReader::set_xvf(const char* type, std::vector<float>& vec)
{
    // check precision of tpr
    if (data_->prec != sizeof(float))
    {
        throw std::runtime_error("Unsupport double precision of tpr in set_xvf");
    }

    // check input type, must X, or V or F or box
    VecProps evec;
    if ((evec = check_string<VecProps>(type, c_mdp_vector)) == VecProps::Count)
    {
        throw std::runtime_error(std::string("Unknown set vector property: ") + type);
    }

    // check vector size 
    if (evec != VecProps::box && evec != VecProps::ef && 
        (int)vec.size() != data_->natoms * DIM)
    {
        throw std::runtime_error("Input vector size is not equal to natoms * 3");
    }

    // check box size
    if (evec == VecProps::box && (int)vec.size() != DIM * DIM)
    {
        throw std::runtime_error("Input box size is not equal to 9");
    }

    // check electric field
    if (evec == VecProps::ef && (int)vec.size() != DIM * 4)
    {
        throw std::runtime_error("Input electric field size is not equal to 12");
    }

    switch (evec)
    {
    case VecProps::x:
    {
        // check if has coordinates of tpr
        if (!data_->bX)
        {
            throw std::runtime_error("Input tpr has not coordinates information");
        }
        // if succeed, return 
        if (write_xvf(vec, data_->property.x, data_->prec)) return TPR_SUCCESS;
        break;
    }
    case VecProps::v:
    {
        // check if has velocity of tpr
        if (!data_->bV)
        {
            throw std::runtime_error("Input tpr has not velocity information");
        }
        if (write_xvf(vec, data_->property.v, data_->prec)) return TPR_SUCCESS;
        break;
    }
    case VecProps::f:
    {
        // check if has force of tpr
        if (!data_->bF)
        {
            throw std::runtime_error("Input tpr has not force information");
        }
        if (write_xvf(vec, data_->property.f, data_->prec)) return TPR_SUCCESS;
        break;
    }
    case VecProps::box:
    {
        if (!data_->bBox)
        {
            throw std::runtime_error("Input tpr has not box information");
        }
        if (write_xvf(vec, data_->property.box, data_->prec)) return TPR_SUCCESS;
        break;
    }
    case VecProps::ef:
    {
        if (write_ef(vec, data_->property.ef, data_->prec)) return TPR_SUCCESS;
        break;
    }
    default:
        throw std::invalid_argument(std::string("Unknown set keyword: ") + type);
        break;
    }

    return TPR_FAILED;
}
