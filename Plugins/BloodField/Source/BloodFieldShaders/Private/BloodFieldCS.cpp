#include "BloodFieldCS.h"

IMPLEMENT_GLOBAL_SHADER(
	FBloodSelectCS,
	"/Plugin/BloodField/Private/BloodField.usf",
	"SelectionPassCS",
	SF_Compute
);

IMPLEMENT_GLOBAL_SHADER(
	FBloodFieldResolveCS,
	"/Plugin/BloodField/Private/BloodField.usf",
	"ResolvePassCS",
	SF_Compute
);