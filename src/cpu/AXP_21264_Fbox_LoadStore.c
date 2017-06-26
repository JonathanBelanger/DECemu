/*
 * Copyright (C) Jonathan D. Belanger 2017.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied only
 * in accordance with the terms of such license and with the inclusion of the
 * above copyright notice.  This software or any other copies thereof may not
 * be provided or otherwise made available to any other person.  No title to
 * and ownership of the software is hereby transferred.
 *
 * The information in this software is subject to change without notice and
 * should not be construed as a commitment by the author or co-authors.
 *
 * The author and any co-authors assume no responsibility for the use or
 * reliability of this software.
 *
 * Description:
 *
 *	This source file contains the functions needed to implement the
 *	functionality of the Fbox Load/Store Instructions.
 *
 *	Revision History:
 *
 *	V01.000		24-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		25-Jun-2017	Jonathan D. Belanger
 *	Updated to used a structured floating-point register and memory format, and
 *	not bit masks and shifts.
 */
#include "AXP_21264_Fbox_LoadStore.h"

/*
 * IMPLEMENTATION NOTES:
 *
 * 		1)	If R31 is a destination register, then the code that selects the
 * 			instruction for execution from the FQ, will determine this and just
 * 			move the instruction state to WaitingRetirement.  The exception to
 * 			this are the LDS and LDT instructions, where these instructions
 * 			become PREFETCH_M and PREFETCH_MEN, respectively.
 * 		2)	When these functions are called, the instruction state is set to
 * 			Executing prior to the call.
 */

/*
 * AXP_LDF
 *	This function implements the Load VAX F Format from Memory to Register
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;
	u64 tmp, exp;
	AXP_F_MEMORY *tmpF = (AXP_F_MEMORY *) &tmp;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v.r.uq + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_LONG(va);

	/*
	 * Get the value out of memory (it'll be in memory format and is 32-bits)
	 */
	tmp = (vaPrime);		// TODO: Load from mem/cache

	/*
	 * Extract the exponent, then expand it from 8-bits to 11-bits.
	 */
	exp = tmpF->exponent;
	if (exp != 0)
		exp += (AXP_G_BIAS - AXP_F_BIAS);

	/*
	 * Now put everything back together, but this time in register format and
	 * 64-bits.
	 */
	instr->destv.fp.fCvt.sign = tmpF->sign;
	instr->destv.fp.fCvt.exponent = exp;
	instr->destv.fp.fCvt.fractionHigh = tmpF->fractionHigh;
	instr->destv.fp.fCvt.fractionLow = tmpF->fractionLow;
	instr->destv.fp.fCvt.zero = 0;

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_LDG
 *	This function implements the Load VAX G Format from Memory to Register
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;
	u64 tmp;
	AXP_G_MEMORY *tmpG = (AXP_G_MEMORY *) &tmp;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v.r.uq + instr->displacement;

	/*
	 * Get the value out of memory (it'll be in memory format and is 64-bits)
	 */
	tmp = (va);		// TODO: Load from mem/cache

	/*
	 * Now put everything back together, but this time in register format and
	 * 64-bits.
	 */
	instr->destv.fp.gCvt.sign = tmpG->sign;
	instr->destv.fp.gCvt.exponent = tmpG->exponent;
	instr->destv.fp.gCvt.fractionHigh = tmpG->fractionHigh;
	instr->destv.fp.gCvt.fractionMidHigh = tmpG->fractionMidHigh;
	instr->destv.fp.gCvt.fractionMidLow = tmpG->fractionMidLow;
	instr->destv.fp.gCvt.fractionLow = tmpG->fractionLow;

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_LDS
 *	This function implements the Load/Prefetch IEEE S Format from Memory
 *	to Register/no-where instruction of the Alpha AXP processor.
 *
 *	If the destination register is F31, then this instruction becomes the
 *	PREFETCH_EN instruction.
 *
 *	A prefetch is a hint to the processor that a cache block might be used in
 *	the future and should be brought into the cache now.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;
	u64 tmp,exp;
	AXP_S_MEMORY *tmpS = (AXP_S_MEMORY *) &tmp;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v.r.uq + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_LONG(va);

	/*
	 * Get the value out of memory (it'll be in memory format and is 32-bits)
	 */
	tmp = (vaPrime);		// TODO: Load from mem/cache

	/*
	 * Extract the exponent, then expand it from 8-bits to 11-bits.
	 */
	exp = tmpS->exponent;
	if (exp == AXP_S_NAN)
		exp = AXP_R_NAN;
	else if (exp != 0)
		exp += (AXP_T_BIAS - AXP_S_BIAS);

	/*
	 * Now put everything back together, but this time in register format and
	 * 64-bits.
	 */
	instr->destv.fp.sCvt.sign = tmpS->sign;
	instr->destv.fp.sCvt.exponent = exp;
	instr->destv.fp.sCvt.fraction = tmpS->fraction;
	instr->destv.fp.sCvt.zero = 0;

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_LDT
 *	This function implements the Load/Prefetch IEEE T Format from Memory
 *	to Register/no-where instruction of the Alpha AXP processor.
 *
 *	If the destination register is F31, then this instruction becomes the
 *	PREFETCH_MEN instruction.
 *
 *	A prefetch is a hint to the processor that a cache block might be used in
 *	the future and should be brought into the cache now.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v.r.uq + instr->displacement;

	/*
	 * Get the value out of memory (it'll be in memory format and is 64-bits)
	 */
	instr->destv.fp.uq = (va);		// TODO: Load from mem/cache

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STF
 *	This function implements the Store VAX F Format from Register to Memory
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	AXP_F_MEMORY tmpF;
	u64 va, vaPrime;
	u64 exp;
	u64 *tmp = (u64 *) &tmpF;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v.r.uq + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_LONG(va);

	/*
	 * Extract the exponent, then compress it from 11-bits to 8-bits.
	 */
	exp = instr->src1v.fp.f.exponent;
	if (exp != 0)
		exp = exp - AXP_G_BIAS + AXP_F_BIAS;

	/*
	 * Now put everything back together, but this time in memory format and
	 * 32-bits.
	 */
	tmpF.sign = instr->src1v.fp.fCvt.sign;
	tmpF.exponent = exp;
	tmpF.fractionHigh = instr->src1v.fp.fCvt.fractionHigh;
	tmpF.fractionLow = instr->src1v.fp.fCvt.fractionLow;
	// (vaPrime) = tmp;	TODO: Store this into memory.

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Write)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STG
 *	This function implements the Store VAX G Format from Register to Memory
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	AXP_G_MEMORY tmpG;
	u64 va;
	u64 *tmp = (u64 *) &tmpG;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v.r.uq + instr->displacement;

	/*
	 * Now put everything back together, but this time in memory format and
	 * 64-bits.
	 */
	tmpG.sign = instr->src1v.fp.gCvt.sign;
	tmpG.exponent = instr->src1v.fp.gCvt.exponent;
	tmpG.fractionHigh = instr->src1v.fp.gCvt.fractionHigh;
	tmpG.fractionMidHigh = instr->src1v.fp.gCvt.fractionMidHigh;
	tmpG.fractionMidLow = instr->src1v.fp.gCvt.fractionMidLow;
	tmpG.fractionLow = instr->src1v.fp.gCvt.fractionLow;
	// (va) =	tmp;	TODO: Store this into memory

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Write)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STS
 *	This function implements the Store IEEE S Format from Register
 *	to Memory instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	AXP_S_MEMORY tmpS;
	u64 va, vaPrime;
	u64 exp;
	u64 *tmp = (u64 *) &tmpS;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v.r.uq + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_LONG(va);

	/*
	 * Extract the exponent, then compress it from 11-bits to 8-bits.
	 */
	exp = instr->src1v.fp.sCvt.exponent;
	if (exp == AXP_R_NAN)
		exp = AXP_S_NAN;
	else if (exp != 0)
		exp = exp - AXP_T_BIAS + AXP_S_BIAS;

	/*
	 * Now put everything back together, but this time in memory format and
	 * 32-bits.
	 */
	tmpS.sign = instr->src1v.fp.sCvt.sign;
	tmpS.exponent = exp;
	tmpS.fraction = instr->src1v.fp.sCvt.fraction;
	// (vaPrime) = tmp;	TODO: store this into memory

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Write)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STT
 *	This function implements the Store IEEE T Format from Register
 *	to Memory instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v.r.uq + instr->displacement;

	/*
	 * Put the value into memory (it'll be in memory format and is 64-bits)
	 */
	// (va) = instr->src1v.fp.uq;		TODO: Load from mem/cache

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}