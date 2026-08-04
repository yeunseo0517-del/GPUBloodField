#include "BloodFieldCS.h"

IMPLEMENT_GLOBAL_SHADER(
	FBloodFieldCS,
	"/Plugin/BloodField/Private/BloodField.usf",
	"BloodField",
	SF_Compute
);