//
// FILE: nfgsolng.cc -- definition of the class dealing with the GUI part of
//                      the normal form solutions.
//
// $Id$
//

#include "wx/wx.h"
#include "guishare/wxmisc.h"
#include "guishare/wxstatus.h"
#include "base/base.h"
#include "gnullstatus.h"
#include "gpoly.h"
#include "nfgsolng.h"
#include "nfgconst.h"
#include "gfunc.h"

//=========================================================================
//                     guiNfgSolution: Member functions
//=========================================================================

guiNfgSolution::guiNfgSolution(NfgShow *p_parent)
  : m_parent(p_parent), m_traceFile(&gnull), m_traceLevel(0)
{ }

#include "nfstrat.h"
#include "nfdom.h"

NFSupport guiNfgSolution::Eliminate(const NFSupport &p_support)
{
  if (!m_eliminate) {
    return p_support;
  }

  NFSupport ret(p_support);
  const Nfg &nfg = p_support.Game();

  gArray<int> players(nfg.NumPlayers());
  for (int i = 1; i <= nfg.NumPlayers(); i++) 
    players[i] = i;

  if (m_eliminateAll) {
    if (m_eliminateMixed) {
      NFSupport *oldSupport = new NFSupport(ret), *newSupport;
      gNullStatus status;
      while ((newSupport = oldSupport->MixedUndominated(!m_eliminateWeak,
							precRATIONAL,
							players, gnull, status)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      ret = *oldSupport;
      delete oldSupport;
    }
    else {
      NFSupport *oldSupport = new NFSupport(ret), *newSupport;
      gNullStatus status;
      while ((newSupport = oldSupport->Undominated(!m_eliminateWeak,
						   players, gnull, status)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      ret = *oldSupport;
      delete oldSupport;
    }
  }
  else {
    if (m_eliminateMixed) {
      NFSupport *newSupport;
      gNullStatus status;
      if ((newSupport = ret.MixedUndominated(!m_eliminateWeak,
					     precRATIONAL,
					     players, gnull, status)) != 0) {
	ret = *newSupport;
	delete newSupport;
      }
    }
    else {
      NFSupport *newSupport;
      gNullStatus status;
      if ((newSupport = ret.Undominated(!m_eliminateWeak,
					players, gnull, status)) != 0) {
	ret = *newSupport;
	delete newSupport;
      }
    }
  }

  return ret;
}

//=========================================================================
//                     Derived classes, by algorithm
//=========================================================================

//-------------
// QreGrid
//-------------

#include "dlqregrid.h"
#include "nfgqregrid.h"

guinfgQreAll::guinfgQreAll(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

gList<MixedSolution> guinfgQreAll::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "QreGridSolve Progress");

  QreNfgGrid qre;
  qre.SetMinLambda(m_minLam);
  qre.SetMaxLambda(m_maxLam);
  qre.SetDelLambda(m_delLam);
  qre.SetTol1(m_tol1);
  qre.SetTol2(m_tol2);
  qre.SetDelP1(m_delp1);
  qre.SetDelP2(m_delp2);
  qre.SetPowLambda(m_powLam);

  gList<MixedSolution> solutions;
  try {
    qre.Solve(p_support, *m_pxiFile, status, solutions);
  }
  catch (gSignalBreak &) { }
  
  if (m_runPxi) {
    if (!wxExecute((char *) (m_pxiCommand + " " + m_pxiFilename))) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }
  return solutions;
}

bool guinfgQreAll::SolveSetup(void)
{
  dialogQreGrid dialog(m_parent, m_parent->Filename().c_str());

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_minLam = dialog.MinLam();
    m_maxLam = dialog.MaxLam();
    m_delLam = dialog.DelLam();
    m_tol1 = dialog.Tol1();
    m_tol2 = dialog.Tol2();
    m_delp1 = dialog.DelP1();
    m_delp2 = dialog.DelP2();
    m_powLam = (dialog.LinearPlot()) ? 0 : 1;
    m_multiGrid = dialog.MultiGrid();

    m_pxiFile = dialog.PxiFile();
    m_pxiFilename = dialog.PxiFilename();
    m_runPxi = dialog.RunPxi();
    m_pxiCommand = dialog.PxiCommand();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}



