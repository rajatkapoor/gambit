//
// FILE: nfgiter.c -- Implementation of normal form iterators
//
// $Id$
//

#include "nfgiter.h"
#include "nfg.h"
#include "nfstrat.h"
#include "nfplayer.h"
#include "nfgciter.h"

//--------------------------------------------------------------------------
// NfgIter:  Constructors, Destructors, Operators
//--------------------------------------------------------------------------

NfgIter::NfgIter(Nfg &nfg)
  : support(nfg),
    N(&nfg), current_strat(nfg.NumPlayers()), profile(nfg)
{
  First();
}

NfgIter::NfgIter(const NFSupport &s) 
  : support(s), N((Nfg *) &s.Game()),
    current_strat(N->NumPlayers()), profile(*N)
{
  First();
}

NfgIter::NfgIter(const NfgIter &it)
  : support(it.support), N(it.N), current_strat(it.current_strat), 
    profile(it.profile)
{ }

NfgIter::NfgIter(const NfgContIter &it)
  : support(it.support), N(it.N), current_strat(it.current_strat),
    profile(it.profile)
{ }

NfgIter::~NfgIter()
{ }

NfgIter &NfgIter::operator=(const NfgIter &it)
{
  if (this != &it)  {
    N = it.N;
    profile = it.profile;
    current_strat = it.current_strat;
    support = it.support;
  }
  return *this;
}

//-----------------------------
// NfgIter: Member Functions
//-----------------------------

void NfgIter::First(void)
{
  for (int i = 1; i <= N->NumPlayers(); i++)  {
    Strategy *s = support.Strategies(i)[1];
    profile.Set(i, s);
    current_strat[i] = 1;
  }
}

int NfgIter::Next(int p)
{
  Strategy *s;
  if (current_strat[p] < support.NumStrats(p))  {
    s = support.Strategies(p)[++(current_strat[p])];
    profile.Set(p, s);
    return 1;
  }
  s = support.Strategies(p)[1];
  profile.Set(p, s);
  current_strat[p] = 1;
  return 0;
}

int NfgIter::Set(int p, int s)
{
  if (p <= 0 || p > N->NumPlayers() ||
      s <= 0 || s > support.NumStrats(p))
    return 0;
  
  profile.Set(p, support.Strategies(p)[s]);
  return 1;
}

void NfgIter::Get(gArray<int> &t) const
{
  for (int i = 1; i <= N->NumPlayers(); i++)
    t[i] = profile[i]->Number();
}

void NfgIter::Set(const gArray<int> &t)
{
  for (int i = 1; i <= N->NumPlayers(); i++){
    profile.Set(i, support.Strategies(i)[t[i]]);
    current_strat[i] = t[i];
  } 
}

long NfgIter::GetIndex(void) const
{
  return profile.GetIndex();
}

NFOutcome *NfgIter::GetOutcome(void) const
{
  return N->GetOutcome(profile);
}

void NfgIter::SetOutcome(NFOutcome *outcome)
{
  N->SetOutcome(profile, outcome);
}


//-------------------------------------
// NfgContIter: Constructor, Destructor
//-------------------------------------

NfgContIter::NfgContIter(const NFSupport &s)
  : support(s), 
    current_strat(s.Game().NumPlayers()),
    N((Nfg *) &s.Game()), profile(*N), thawed(N->NumPlayers())
{
  for (int i = 1; i <= thawed.Length(); i++)
    thawed[i] = i;

  First();
}

NfgContIter::~NfgContIter()
{ }

//------------------------------
// NfgContIter: Member Functions
//------------------------------

void NfgContIter::First(void)
{
  for (int i = 1; i <= thawed.Length(); i++){
    profile.Set(thawed[i], support.Strategies(thawed[i])[1]);
    current_strat[thawed[i]] = 1;
  }	
}

void NfgContIter::Set(int pl, int num)
{
  if (!frozen.Contains(pl))   return;

  profile.Set(pl, support.Strategies(pl)[num]);
  current_strat[pl] = num;
}

void NfgContIter::Set(const Strategy *s)
{
  if (!frozen.Contains(s->Player()->GetNumber()))   return;

  profile.Set(s->Player()->GetNumber(), s);
  current_strat[s->Player()->GetNumber()] = s->Number();
}

void NfgContIter::Freeze(const gBlock<int> &freeze)
{
  frozen = freeze;
  thawed = gBlock<int>(N->NumPlayers() - freeze.Length());
  for (int i = 1, j = 1; i <= N->NumPlayers(); i++)
    if (!frozen.Contains(i))   thawed[j++] = i;
  First();
}

void NfgContIter::Freeze(int pl)
{
  if (frozen.Contains(pl))   return;
  frozen.Append(pl);
  thawed.Remove(thawed.Find(pl));
  First();
}

void NfgContIter::Thaw(int pl)
{
  if (thawed.Contains(pl))   return;
  frozen.Remove(frozen.Find(pl));
  int i = 1;
  while (thawed[i] < pl)   i++;
  thawed.Insert(pl, i);
  First();
}

int NfgContIter::NextContingency(void)
{
  int j = thawed.Length();
  if (j == 0) return 0;    	

  Strategy *s;
  while (1)   {
    int pl = thawed[j];
    if (current_strat[pl] < support.NumStrats(pl)) {
      s = support.Strategies(pl)[++(current_strat[pl])];
      profile.Set(pl, s);
      return 1;
    }
    profile.Set(pl, support.Strategies(pl)[1]);
    current_strat[pl] = 1;
    j--;
    if (j == 0)
      return 0;
  }
}

long NfgContIter::GetIndex(void) const
{
  return profile.GetIndex();
}

const StrategyProfile &NfgContIter::Profile(void) const
{
  return profile;
}

gArray<int> NfgContIter::Get(void) const
{
  gArray<int> current(N->NumPlayers());
  for (int i = 1; i <= current.Length(); i++)
    current[i] = profile[i]->Number();
  return current;
}

void NfgContIter::Get(gArray<int> &t) const
{
  for (int i = 1; i <= N->NumPlayers(); i++)
    t[i] = profile[i]->Number();
}

NFOutcome *NfgContIter::GetOutcome(void) const
{
  return N->GetOutcome(profile);
}

void NfgContIter::SetOutcome(NFOutcome *outcome)
{
  N->SetOutcome(profile, outcome);
}

void NfgContIter::Dump(gOutput &f) const
{
  f << "{ ";
  for (int i = 1; i <= N->NumPlayers(); i++)
    f << profile[i]->Number() << ' ';
  f << '}';
}

