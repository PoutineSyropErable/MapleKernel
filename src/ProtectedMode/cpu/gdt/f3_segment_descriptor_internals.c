#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bit_hex_string.h"
#include "f1_binary_operation.h"
#include "f3_segment_descriptor_internals.h"
#include "gdt.h"
#include "vga_terminal.h"
#include <stdbool.h>

uint8_t getType(SegmentDescriptor *sd)
{

	SegmentDescriptorModern *sdm = (SegmentDescriptorModern *)sd;
	return sdm->type;
}

void setType(SegmentDescriptor *sd, uint8_t type)
{

	SegmentDescriptorModern *sdm = (SegmentDescriptorModern *)sd;
	sdm->type					 = type;

	printBinary(type, "type");
}

uint8_t getDescriptorTypeS(SegmentDescriptor *sd)
{

	uint8_t S = getBits(sd->higher, DESCRIPTOR_TYPE_S, DESCRIPTOR_TYPE_S);
	return S;
}

void setDescriptorTypeS(SegmentDescriptor *sd, uint8_t descriptorTypeS)
{

	setBitsModify(&sd->higher, descriptorTypeS, DESCRIPTOR_TYPE_S, DESCRIPTOR_TYPE_S);
	printBinary(descriptorTypeS, "descriptorTypeS");
}

// Descriptor Priviledge Level
uint8_t getPriviledgeDPL(SegmentDescriptor *sd)
{

	uint8_t DPL = getBits(sd->higher, DPL_END, DPL_START);
	return DPL;
}

// Descriptor Priviledge Level
void setPriviledgeDPL(SegmentDescriptor *sd, uint8_t priviledgeDPL)
{

	setBitsModify(&sd->higher, priviledgeDPL, DPL_END, DPL_START);

	printBinary(priviledgeDPL, "priviledgeDPL");
}

uint8_t getPresent(SegmentDescriptor *sd)
{

	uint8_t P = getBits(sd->higher, PRESENT, PRESENT);
	return P;
}

void setPresent(SegmentDescriptor *sd, uint8_t presentP)
{

	setBitsModify(&sd->higher, presentP, PRESENT, PRESENT);
	printBinary(presentP, "presentP");
}

uint32_t getSegmentLimit(SegmentDescriptor *sd)
{

	uint32_t segmentLimit_19_16 = getBits(sd->higher, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);
	uint32_t segmentLimit_15_0	= getBits(sd->lower, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	uint32_t segmentLimt_19_16_shifted = segmentLimit_19_16 << 16;
	return segmentLimt_19_16_shifted | segmentLimit_15_0;
}

void setSegmentLimit(SegmentDescriptor *sd, uint32_t segmentLimit)
{

	uint32_t higher_19_16 = getBits(segmentLimit, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);
	uint32_t lower_15_0	  = getBits(segmentLimit, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	setBitsModify(&sd->higher, higher_19_16, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);
	setBitsModify(&sd->lower, lower_15_0, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	printBinary(segmentLimit, "segmentLimit");
	// printBinary(higher_19_16, "higher_19_16");
	// printBinary(lower_15_0, "lower_15_0");
}

// Available for system software
uint8_t getAVL(SegmentDescriptor *sd)
{

	uint8_t avl = getBits(sd->higher, AVL, AVL);
	return avl;
}

void setAVL(SegmentDescriptor *sd, uint8_t avl)
{

	setBitsModify(&sd->higher, avl, AVL, AVL);
	printBinary(avl, "avl");
}

uint8_t getLongMode(SegmentDescriptor *sd)
{

	uint8_t L = getBits(sd->higher, LONG_MODE, LONG_MODE);
	return L;
}

void setLongMode(SegmentDescriptor *sd, uint8_t L)
{

	setBitsModify(&sd->higher, L, LONG_MODE, LONG_MODE);
	printBinary(L, "L");
}

uint8_t getDefaultOperationSize(SegmentDescriptor *sd)
{

	// D slash B. D/B. D or B
	uint8_t DsB = getBits(sd->higher, DEFAULT_OPERATION_SIZE, DEFAULT_OPERATION_SIZE);
	return DsB;
}

void setDefaultOperationSize(SegmentDescriptor *sd, uint8_t DsB)
{

	setBitsModify(&sd->higher, DsB, DEFAULT_OPERATION_SIZE, DEFAULT_OPERATION_SIZE);
	printBinary(DsB, "DsB");
}

uint8_t getGranularity(SegmentDescriptor *sd)
{

	uint8_t granularity = getBits(sd->higher, GRANULARITY, GRANULARITY);
	return granularity;
}

void setGranularity(SegmentDescriptor *sd, uint8_t granularity)
{

	setBitsModify(&sd->higher, granularity, GRANULARITY, GRANULARITY);
	printBinary(granularity, "granularity");
}

uint32_t getBaseAddress(SegmentDescriptor *sd)
{
	/*
	Using magic values cause it somehow cleaner
	*/

	uint32_t base_0_15	= getBits(sd->lower, 31, 16);
	uint32_t base_16_23 = getBits(sd->higher, 7, 0);
	uint32_t base_24_31 = getBits(sd->higher, 31, 24);

	uint32_t base_0_15_shifted	= base_0_15 << 0; // get optimised. But i prefer clarity
	uint32_t base_16_23_shifted = base_16_23 << 16;
	uint32_t base_24_31_shifted = base_24_31 << 24;

	uint32_t baseAddress = base_0_15_shifted | base_16_23_shifted | base_24_31_shifted;

	return baseAddress;
}

void setBaseAddress(SegmentDescriptor *sd, uint32_t baseAddress)
{

	uint32_t higher_31_24 = getBits(baseAddress, 31, 24);
	uint32_t higher_7_0	  = getBits(baseAddress, 23, 16);
	uint32_t lower_31_16  = getBits(baseAddress, 15, 0);

	setBitsModify(&sd->lower, lower_31_16, 31, 16);
	setBitsModify(&sd->higher, higher_7_0, 7, 0);
	setBitsModify(&sd->higher, higher_31_24, 31, 24);

	printBinary(baseAddress, "baseAddress");
	// printf("\n");
	// printBinary(higher_31_24, "higher_31_24 (Base 31-24)"); // 8
	// printBinary(higher_7_0, "higher_7_0 (Base 23-16)  ");   // 8
	//
	// printBinary(lower_31_16, "lower_31_16 (Base 15-0)  "); // 8
	//
	// printBinary(sd->lower, "sd->lower");   // 8
	// printBinary(sd->higher, "sd->higher"); // 8

	return;
}

/* ======================= EXTRAS, USED FOR OTHER THINGS ======================== */

uint8_t get_access(SegmentDescriptor *sd)
{
	SegmentDescriptorNice *sdNice = (SegmentDescriptorNice *)sd;

	uint8_t access = sdNice->p_dpl_s_type;
	return access;
}

void terminal_writestring(const char *data);

uint8_t get_flags(SegmentDescriptor *sd)
{
	SegmentDescriptorNice *sdNice = (SegmentDescriptorNice *)sd;

	uint8_t					 flags_segmentLimitHigh = sdNice->g_db_l_avl_segmentLimitHigh;
	uint8_t					 flags					= flags_segmentLimitHigh >> 4;
	[[maybe_unused]] uint8_t segmentLimitHigh		= flags_segmentLimitHigh & 0xF;

	if (flags > 0xF)
	{
		terminal_writestring("Flags is too big, this is an impossible case\n");
	}
	return flags;
}

void printFlags(SegmentDescriptor *sd)
{
	uint8_t flags = get_flags(sd);
	char	flagsOutput[INTERPRET_STR_LEN];
	printBinarySize(flags, "flags", 4);
	interpret_flags(flags, flagsOutput);
	terminal_writestring(flagsOutput);
}

void printAccess(SegmentDescriptor *sd)
{
	uint8_t access = get_access(sd);
	char	accessOutput[INTERPRET_STR_LEN];
	printBinarySize(access, "access", 8);
	interpret_access(access, accessOutput);
	terminal_writestring(accessOutput);
}

char bool_to_char(bool b)
{
	if (b == 1)
	{
		return '1';
	}
	else
	{
		return '0';
	}
}

void modify_string(char *str, uint8_t value)
{
	char					*c			  = str;
	[[maybe_unused]] uint8_t string_index = 0;
	uint8_t					 bit_index	  = 0;
	while (*c != '\0')
	{

		if (*c == '%')
		{
			if (bit_index >= 8)
			{
				break;
			}
			bool c_bool = (value >> bit_index) & 1;
			*c			= bool_to_char(c_bool);
			bit_index++;
		}

		string_index++;
		c++;
	}
}

void modify_string_opsize(char *str, uint8_t value)
{
	char   *c			 = str;
	uint8_t string_index = 0;
	uint8_t digit_index	 = 0;

	char value_str[3];
	value_str[2] = '\0';
	switch (value)
	{
	case 16:
		value_str[0] = '1';
		value_str[1] = '6';
		break;
	case 32:
		value_str[0] = '3';
		value_str[1] = '2';
		break;
	case 64:
		value_str[0] = '6';
		value_str[1] = '4';
		break;
	}

	while (*c != '\0')
	{

		if (*c == '%')
		{
			if (digit_index >= 2)
			{
				break;
			}
			str[string_index] = value_str[digit_index];
			digit_index++;
		}

		string_index++;
		c++;
	}
}

typedef struct
{
	char *header_str;
	char *present_str;
	char *dpl_str;
	char *not_sys_str;
	char *exec_str;
	char *conforming_str;
	char *read_write_str;
	char *accessed_str;
} AccessDesc;

typedef struct
{
	char *header_str;
	char *granularity_str;
	char *opsize_32_str;
	char *long_mode_str;
	char *avail_str;
	char *opsize_str;
} FlagsDesc;

enum DescTag
{
	Access = 0,
	Flags  = 1,

};

typedef struct
{
	enum DescTag tag;
	union
	{
		AccessDesc accessDesc;
		FlagsDesc  flagsDesc;
	} desc;
} TaggedDesc;

void create_access_desc_str(char *output, TaggedDesc taggedDesc)
{
	char   *header_str;
	char  **fields;
	uint8_t field_count;

	switch (taggedDesc.tag)
	{
	case Access:
		AccessDesc access_desc = taggedDesc.desc.accessDesc;
		header_str			   = access_desc.header_str;
		fields				   = (char **)&access_desc.present_str;
		field_count			   = 7;
		break;
	case Flags:
		FlagsDesc flagsDesc = taggedDesc.desc.flagsDesc;
		header_str			= flagsDesc.header_str;
		fields				= (char **)&flagsDesc.granularity_str;
		field_count			= 5;
		break;
	}

	uint8_t char_index = 0;

	char *header_char = header_str;
	while (*header_char != '{')
	{
		output[char_index] = *header_char;
		char_index++;
		header_char++;
	}
	output[char_index] = '{';
	char_index++;
	header_char++;

	for (uint8_t field_index = 0; field_index < field_count; field_index++)
	{
		char *chosen_field		= fields[field_index];
		char *chosen_field_char = chosen_field;
		output[char_index]		= '\n';
		output[char_index + 1]	= '\t';
		char_index += 2;
		while (*chosen_field_char != '\0')
		{
			output[char_index] = *chosen_field_char;
			char_index++;
			chosen_field_char++;
		}
	}
	output[char_index] = '\n';
	char_index++;

	while (*header_char != '\0')
	{
		output[char_index] = *header_char;
		char_index++;
		header_char++;
	}
	output[char_index]	   = '\n';
	output[char_index + 1] = '\0';
}

void interpret_access(uint8_t access, char *output)
{

	bool	present				   = (access >> 7) & 1;
	uint8_t dpl					   = (access >> 5) & 0b11;
	bool	not_sys				   = (access >> 4) & 1;
	bool	exec				   = (access >> 3) & 1;
	bool	conforming_expand_down = (access >> 2) & 1;
	bool	read_write			   = (access >> 1) & 1;
	bool	accessed			   = (access >> 0) & 1;

	AccessDesc access_desc;
	char	   header_str[] = "Access = {}";
	access_desc.header_str	= header_str;
	if (exec)
	{
		// type = code.

		char present_str[] = "present = %";
		modify_string(present_str, present);
		access_desc.present_str = present_str;

		char dpl_str[] = "dpl = %%";
		modify_string(dpl_str, dpl);
		access_desc.dpl_str = dpl_str;

		char not_sys_str[] = "Not System = %";
		modify_string(not_sys_str, not_sys);
		access_desc.not_sys_str = not_sys_str;

		// The type
		char exec_str[]		 = "Code";
		access_desc.exec_str = exec_str;

		char conforming_str[] = "Non Conforming = %";
		modify_string(conforming_str, conforming_expand_down);
		access_desc.conforming_str = conforming_str;

		char read_write_str[] = "Read Permission = %";
		modify_string(read_write_str, read_write);
		access_desc.read_write_str = read_write_str;

		char accessed_str[] = "accessed = %";
		modify_string(accessed_str, accessed);
		access_desc.accessed_str = accessed_str;

		TaggedDesc taggedDesc;
		taggedDesc.tag			   = Access;
		taggedDesc.desc.accessDesc = access_desc;
		create_access_desc_str(output, taggedDesc);
	}
	else
	{
		char present_str[] = "present = %";
		modify_string(present_str, present);
		access_desc.present_str = present_str;

		char dpl_str[] = "dpl = %%";
		modify_string(dpl_str, dpl);
		access_desc.dpl_str = dpl_str;

		char not_sys_str[] = "Not System = %";
		modify_string(not_sys_str, not_sys);
		access_desc.not_sys_str = not_sys_str;

		// Type
		char exec_str[]		 = "Data";
		access_desc.exec_str = exec_str;

		char conforming_str[] = "Expand Down = %";
		modify_string(conforming_str, conforming_expand_down);
		access_desc.conforming_str = conforming_str;

		char read_write_str[] = "Write Permission = %";
		modify_string(read_write_str, read_write);
		access_desc.read_write_str = read_write_str;

		char accessed_str[] = "accessed = %";
		modify_string(accessed_str, accessed);
		access_desc.accessed_str = accessed_str;

		TaggedDesc taggedDesc;
		taggedDesc.tag			   = Access;
		taggedDesc.desc.accessDesc = access_desc;
		create_access_desc_str(output, taggedDesc);
	}
}

void interpret_flags(uint8_t flags, char *output)
{
	bool granularity = (flags >> 7) & 1;
	bool opsize_32	 = (flags >> 6) & 1;
	bool long_mode	 = (flags >> 5) & 1;
	bool avail		 = (flags >> 4) & 1;

	uint8_t opsize;
	if (long_mode)
	{
		opsize = 64;
	}
	else
	{
		if (opsize_32)
		{
			opsize = 32;
		}
		else
		{
			opsize = 16;
		}
	}

	FlagsDesc flagsDesc;
	char	  header_str[] = "Flags = {}\n";
	flagsDesc.header_str   = header_str;

	char granularity_str[] = "granularity = %";
	modify_string(granularity_str, granularity);
	flagsDesc.granularity_str = granularity_str;

	char opsize_32_str[] = "opsize32 = %";
	modify_string(opsize_32_str, opsize_32);
	flagsDesc.opsize_32_str = opsize_32_str;

	char long_mode_str[] = "long_mode = %";
	modify_string(long_mode_str, long_mode);
	flagsDesc.long_mode_str = long_mode_str;

	char avail_str[] = "avail = %";
	modify_string(avail_str, avail);
	flagsDesc.avail_str = avail_str;

	char opsize_str[] = "___real_opsize = %%";
	modify_string_opsize(opsize_str, opsize);
	flagsDesc.opsize_str = opsize_str;

	TaggedDesc taggedDesc;
	taggedDesc.tag			  = Flags;
	taggedDesc.desc.flagsDesc = flagsDesc;
	create_access_desc_str(output, taggedDesc);
}

void interpret_test(void)
{
	// printf("\n\n=====Start of program=====\n\n");

	char	output[INTERPRET_STR_LEN];
	uint8_t access;
	uint8_t flags;

	// memset(output, 0, INTERPRET_STR_LEN);
	access = PRESENT | NOT_SYS | RW | ACCESSED | EXEC;
	interpret_access(access, output);
	// printf("%s", output);
	// printf("%zu\n", strlen(output));

	// memset(output, 0, INTERPRET_STR_LEN);
	access = PRESENT | NOT_SYS | RW | ACCESSED;
	interpret_access(access, output);
	// printf("%s", output);
	// printf("%zu\n", strlen(output));

	// memset(output, 0, INTERPRET_STR_LEN);
	flags = GRAN_4K;
	interpret_flags(flags, output);
	// printf("%s", output);
	// printf("%zu\n", strlen(output));
}
