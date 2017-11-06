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
 *	This source file contains the main function to test the code dumping code.
 *
 *	Revision History:
 *
 *	V01.000		06-Nov-2017	Jonathan D. Belanger
 *	Initially written.
 *
 */
#include "AXP_Dumps.h"

/*
 * Define some values for 1 and 8 Meg
 */
#define	ONE_M	(ONE_K * ONE_K)
#define EIGHT_M	(ONE_M * 8)

/*
 * Allocate memory inwhich to store the instructions read from a file.
 */
u8	memory[EIGHT_M];

/*
 * AXP_21264_LoadMemory
 *	This funciton is called to open a file containing binary data representing
 *	Alpha AXP code and load that data into the memory buffer.
 *
 * Input Parameters:
 *	fileName:
 *		A string containing the name of the binary file to open.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	Bytes loaded (0 means none/error).
 */
int AXP_21264_LoadMemory(char *fileName)
{
	bool	done = false;
	FILE	*fp = NULL;
	int		ii = 0;

	fp = fopen(fileName, "r");
	if (fp != NULL)
	{
		while ((feof(fp) == 0) && (done == false))
		{
			fread(&memory[ii++], 1, 1, fp);
			if (ii >= EIGHT_M)
			{
				printf(
					"Input file %s is too big for %d Meg of memory.\n",
					fileName,
					(EIGHT_M/ONE_M));
				ii = 0;
				done = true;
			}
		}
		flose(fp);
	}
	else
	{
		printf("Unable to open file: %s\n", fileName);
		ii = 0;
	}
	return(ii);
}

/*
 * main
 *	This function is the main function called to exercise the Digital AXP
 *	Alpha 21264 instruction dump code.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	0 for success
 *	<0 for failure
 */
int main()
{
	char decodedLine[256];
	u32	*instr;
	int	retVal = 0;
	int	totalBytesRead = 0;
	int totalInstructions = 0;
	int ii;
	bool decodedResult = true;

	printf("\nAXP 21264 Instruction Dumping Tester\n");
	totalBytesRead = AXP_21264_LoadMemory("");
	if (totalBytesRead > 0)
	{
		totalInstructions = totalBytesRead / sizeof(u32);
		instr = (u32 *) &memory[0];
		for (ii = 0;
			 ((ii < totalInstructions) && (decodedResult == true));
			 ii++)
		{
			decodedResult = AXP_Decode_Instruction(
								*((AXP_INS_FMT *) &memory[ii]),
								true,
								decodedLine);
			if (decodedResult == true)
				printf("0x%016llx: 0x%08x: %s\n", ii, *((u32 *) &memory[ii]), decodedLine);
		}
	}
	else
		retVal = -1;
	return(retVal);
}