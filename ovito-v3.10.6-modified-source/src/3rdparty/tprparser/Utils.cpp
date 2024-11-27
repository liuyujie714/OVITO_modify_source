#include <stdexcept>
#include <string>

#include "Utils.h"

std::pair<int, std::vector<float>> get_bond_type(int ftype, const t_iparams *param)
{
    std::vector<float> ffparam;
    switch (ftype)
    {
    case F_BONDS: 
        ffparam.push_back(param->harmonic.rA);
        ffparam.push_back(param->harmonic.krA);
        ffparam.push_back(param->harmonic.rB);
        ffparam.push_back(param->harmonic.krB);
        return std::make_pair(1, ffparam);
    case F_G96BONDS:
        ffparam.push_back(param->harmonic.rA);
        ffparam.push_back(param->harmonic.krA);
        ffparam.push_back(param->harmonic.rB);
        ffparam.push_back(param->harmonic.krB);
        return std::make_pair(2, ffparam);
    case F_MORSE:
        ffparam.push_back(param->morse.b0A);
        ffparam.push_back(param->morse.cbA);
        ffparam.push_back(param->morse.betaA);
        ffparam.push_back(param->morse.b0B);
        ffparam.push_back(param->morse.cbB);
        ffparam.push_back(param->morse.betaB);
        return std::make_pair(3, ffparam);
    case F_CUBICBONDS:
        ffparam.push_back(param->cubic.b0);
        ffparam.push_back(param->cubic.kb);
        ffparam.push_back(param->cubic.kcub);
        return std::make_pair(4, ffparam);
    case F_CONNBONDS:
        return std::make_pair(5, ffparam);
    case F_HARMONIC:
        ffparam.push_back(param->harmonic.rA);
        ffparam.push_back(param->harmonic.krA);
        ffparam.push_back(param->harmonic.rB);
        ffparam.push_back(param->harmonic.krB);
        return std::make_pair(6, ffparam);
    case F_FENEBONDS:
        ffparam.push_back(param->fene.bm);
        ffparam.push_back(param->fene.kb);
        return std::make_pair(7, ffparam);
    case F_TABBONDS:
        ffparam.push_back(param->tab.kA);
        ffparam.push_back(static_cast<float>(param->tab.table)); // int to float
        ffparam.push_back(param->tab.kB);
        return std::make_pair(8, ffparam);
    case F_TABBONDSNC:
        ffparam.push_back(param->tab.kA);
        ffparam.push_back(static_cast<float>(param->tab.table)); // int to float
        ffparam.push_back(param->tab.kB);
        return std::make_pair(9, ffparam);
    case F_RESTRBONDS:
        ffparam.push_back(param->restraint.lowA);
        ffparam.push_back(param->restraint.up1A);
        ffparam.push_back(param->restraint.up2A);
        ffparam.push_back(param->restraint.kA);
        ffparam.push_back(param->restraint.lowB);
        ffparam.push_back(param->restraint.up1B);
        ffparam.push_back(param->restraint.up2B);
        ffparam.push_back(param->restraint.kB);
        return std::make_pair(10, ffparam);
    // add settle for water
    case F_SETTLE:
        ffparam.push_back(param->settle.doh);
        ffparam.push_back(param->settle.dhh);
        return std::make_pair(1, ffparam);
    // 1. 含有[ constraints ]的部分
    // 2. 有些成键关系会转换成约束Constraint，取决于mdp约束设置，比如h-bonds
    case F_CONSTR:
        ffparam.push_back(param->constr.dA); // 距离
        ffparam.push_back(param->constr.dB);
        return std::make_pair(1, ffparam);
    case F_CONSTRNC:
        ffparam.push_back(param->constr.dA); // 距离
        ffparam.push_back(param->constr.dB);
        return std::make_pair(2, ffparam);
    default:
        break;
    }
    return std::make_pair(-1, ffparam);
}


std::pair<int, std::vector<float>> get_angle_type(int ftype, const t_iparams* param)
{
    std::vector<float> ffparam;
    switch (ftype)
    {
    case F_ANGLES:
        ffparam.push_back(param->harmonic.rA);
        ffparam.push_back(param->harmonic.krA);
        ffparam.push_back(param->harmonic.rB);
        ffparam.push_back(param->harmonic.krB);
        return std::make_pair(1, ffparam);
    case F_G96ANGLES:
        ffparam.push_back(param->harmonic.rA);
        ffparam.push_back(param->harmonic.krA);
        ffparam.push_back(param->harmonic.rB);
        ffparam.push_back(param->harmonic.krB);
        return std::make_pair(2, ffparam);
    case F_CROSS_BOND_BONDS:
        ffparam.push_back(param->cross_bb.r1e);
        ffparam.push_back(param->cross_bb.r2e);
        ffparam.push_back(param->cross_bb.krr);
        return std::make_pair(3, ffparam);
    case F_CROSS_BOND_ANGLES:
        ffparam.push_back(param->cross_ba.r1e);
        ffparam.push_back(param->cross_ba.r2e);
        ffparam.push_back(param->cross_ba.r3e);
        ffparam.push_back(param->cross_ba.krt);
        return std::make_pair(4, ffparam);
    case F_UREY_BRADLEY:
        ffparam.push_back(param->u_b.thetaA);
        ffparam.push_back(param->u_b.kthetaA);
        ffparam.push_back(param->u_b.r13A);
        ffparam.push_back(param->u_b.kUBA);
        ffparam.push_back(param->u_b.thetaB);
        ffparam.push_back(param->u_b.kthetaB);
        ffparam.push_back(param->u_b.r13B);
        ffparam.push_back(param->u_b.kUBB);
        return std::make_pair(5, ffparam);
    case F_QUARTIC_ANGLES:
        ffparam.push_back(param->qangle.theta);
        for (int i = 0; i < 5; i++) ffparam.push_back(param->qangle.c[i]);
        return std::make_pair(6, ffparam);
    case F_TABANGLES:
        ffparam.push_back(param->tab.kA);
        ffparam.push_back(static_cast<float>(param->tab.table)); // int to float
        ffparam.push_back(param->tab.kB);
        return std::make_pair(8, ffparam);
    case F_LINEAR_ANGLES: // the order is different from tpr
        ffparam.push_back(param->linangle.aA);
        ffparam.push_back(param->linangle.klinA);
        ffparam.push_back(param->linangle.aB);
        ffparam.push_back(param->linangle.klinB);
        return std::make_pair(9, ffparam);
    case F_RESTRANGLES:
        ffparam.push_back(param->harmonic.rA);
        ffparam.push_back(param->harmonic.krA);
        return std::make_pair(10, ffparam);
    default:
        break;
    }

    return std::make_pair(-1, ffparam);
}

std::pair<int, std::vector<float>> get_dihedral_type(int ftype, const t_iparams* param)
{
    std::vector<float> ffparam;
    switch (ftype)
    { 
    case F_PDIHS: // 周期性二面角多重
        ffparam.push_back(param->pdihs.phiA);
        ffparam.push_back(param->pdihs.cpA);
        ffparam.push_back(param->pdihs.phiB);
        ffparam.push_back(param->pdihs.cpB);
        ffparam.push_back(static_cast<float>(param->pdihs.mult));
        //return 1;
        return std::make_pair(9, ffparam);
    case F_RBDIHS:
        for (int i = 0; i < 6; i++) ffparam.push_back(param->rbdihs.rbcA[i]);
        for (int i = 0; i < 6; i++) ffparam.push_back(param->rbdihs.rbcB[i]);
        return std::make_pair(3, ffparam);
    case F_FOURDIHS:
        for (int i = 0; i < 6; i++) ffparam.push_back(param->rbdihs.rbcA[i]);
        for (int i = 0; i < 6; i++) ffparam.push_back(param->rbdihs.rbcB[i]);
        return std::make_pair(5, ffparam);
    case F_TABDIHS:
        ffparam.push_back(param->tab.kA);
        ffparam.push_back(static_cast<float>(param->tab.table)); // int to float
        ffparam.push_back(param->tab.kB);
        return std::make_pair(8, ffparam);
    case F_RESTRDIHS:
        ffparam.push_back(param->pdihs.phiA);
        ffparam.push_back(param->pdihs.cpA);
        return std::make_pair(10, ffparam);
    case F_CBTDIHS:
        for (int i = 0; i < 6; i++) ffparam.push_back(param->cbtdihs.cbtcA[i]);
        return std::make_pair(11, ffparam);
    default:
        break;
    }
    return std::make_pair(-1, ffparam);
}

std::pair<int, std::vector<float>> get_improper_type(int ftype, const t_iparams* param)
{
    std::vector<float> ffparam;
    switch (ftype)
    {
    case F_IDIHS:
        ffparam.push_back(param->harmonic.rA);
        ffparam.push_back(param->harmonic.krA);
        ffparam.push_back(param->harmonic.rB);
        ffparam.push_back(param->harmonic.krB);
        return std::make_pair(2, ffparam);
    case F_PIDIHS:
        ffparam.push_back(param->harmonic.rA);
        ffparam.push_back(param->harmonic.krA);
        ffparam.push_back(param->harmonic.rB);
        ffparam.push_back(param->harmonic.krB);
        return std::make_pair(4, ffparam);
    default:
        break;
    }
    return std::make_pair(-1, ffparam);
}

std::pair<int, std::vector<float>> get_nonbonded_type(int ftype, const t_iparams* param)
{
    std::vector<float> ffparam;

    switch (ftype)
    {
    case F_LJ: // I set up functyepe=3
        ffparam.push_back(param->lj.c6);
        ffparam.push_back(param->lj.c12);
        return std::make_pair(3, ffparam);
    case F_LJ14: // [ pairs ], functype 1
        ffparam.push_back(param->lj14.c6A);
        ffparam.push_back(param->lj14.c12A);
        ffparam.push_back(param->lj14.c6B);
        ffparam.push_back(param->lj14.c12B);
        return std::make_pair(1, ffparam);
    default:
        break;
    }
    return std::make_pair(-1, ffparam);
}

FILE* efopen(const char* fname, const char* mod)
{
    FILE* fp = fopen(fname, mod);
    if (!fp)
    {
        throw std::runtime_error(std::string("Can not open/write file: ") + fname);
    }
    return fp;
}
