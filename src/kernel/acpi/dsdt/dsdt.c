/*
 * File: dsdt.c
 * File Created: 10 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/dsdt/dsdt.h"
#include "acpi/table.h"

#include "asm/mmu_arch.h"

#include "debug/debug.h"

#include "mm/ioremap.h"

#include <binary.h>

#define MODULE "dsdt"

extern void hexdump(void *ptr, size_t len, size_t size);

typedef struct
{
    sdt_header_t header;
    uint8_t aml[];
} PACKED dsdt_t;

typedef struct aml_opcode_info
{
    uint8_t opcode;
    size_t operand_size;
    uint8_t operand_count;
    // aml_operand_type_t operands[MAX_AML_OPERANDS];
} aml_opcode_info_t;

dsdt_t *dsdt;

size_t aml_get_ident(uint8_t *code, int start_index, char *ident_out)
{
    size_t ident_index = 0;
    uint8_t byte = code[start_index];
    if ((byte >= 0x41 && byte <= 0x5A) || (byte >= 0x2E && byte <= 0x39) || (byte >= 0x5C && byte <= 0x6E))
    {
        for (int i = start_index; i < dsdt->header.length; i++)
        {
            byte = dsdt->aml[i];
            ident_out[ident_index] = (char)byte;
            ident_index++;
            uint8_t next_byte = dsdt->aml[i + 1];
            if ((next_byte >= 0x41 && next_byte <= 0x5A) || (next_byte >= 0x2E && next_byte <= 0x39) || (next_byte >= 0x5C && next_byte <= 0x6E))
            {
                continue;
            }
            break;
        }
        ident_out[ident_index] = 0;
        // log_debug(NO_MODULE, "ident = \"%s\"", ident_out);
    }
    return ident_index;
}

aml_opcode_info_t aml_opcode(uint8_t *code, int *start_index)
{
    aml_opcode_info_t info;
    info.opcode = code[(*start_index)];
    uint8_t extra_bytes = BIT_GET_RANGE(code[(*start_index) + 1], 6, 7);
    info.operand_size = BIT_GET_RANGE(code[(*start_index) + 1], 0, 5);
    (*start_index)++;
    (*start_index)++;
    (*start_index) += extra_bytes;
    info.operand_size -= 1;
    return info;
}

int aml_parse_byte_data(uint8_t *code, int *start_index, uint8_t *out_data)
{
    if (code[*start_index] == 0x0a)
    {
        (*start_index)++;
        *out_data = code[*start_index];
        (*start_index)++;
        return 0;
    }
    return 1;
}

void dsdt_init(paddr_t dsdt_phys)
{
    ENTER_FUNC("0x%p", dsdt_phys);

    mmu_arch_map(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)dsdt_phys), PAGE_ALIGN_DOWN((paddr_t)dsdt_phys), kernel_data_flags);
    dsdt = (dsdt_t *)dsdt_phys;
    uint32_t size = dsdt->header.length;
    log_debug(MODULE, "size = %u", size);
    mmu_arch_unmap(&kernel_page, PAGE_ALIGN_DOWN((vaddr_t)dsdt_phys));
    dsdt = (dsdt_t *)(ioremap((paddr_t)dsdt_phys, size) + GET_PAGE_OFFSET(dsdt_phys));
    hexdump(dsdt->aml, dsdt->header.length, 16);
    for (int i = 0; i < dsdt->header.length; i++)
    {
        uint8_t byte = dsdt->aml[i];
        if (byte == 0x08)
        {
            i++;
            char ident[255];
            i += aml_get_ident(dsdt->aml, i, ident) - 1;
            log_debug(NO_MODULE, "NAMEOP \"%s\"", ident);
        }
        else if (byte == 0x10)
        {
            log_debug(NO_MODULE, "SCOPE 0x%x", dsdt->aml[i + 1]);
        }
        else if (byte == 0x11)
        {
            log_debug(NO_MODULE, "BufferOp 0x%x", dsdt->aml[i + 1]);
        }
        else if (byte == 0x12)
        {
            log_debug(NO_MODULE, "PackageOp 0x%x at %u", dsdt->aml[i + 1], i);
            hexdump(dsdt->aml + i, 16, 16);
            aml_opcode_info_t info = aml_opcode(dsdt->aml, &i);
            log_debug(NO_MODULE, "PackageOp 0x%x at %u", dsdt->aml[i], i);
            uint8_t num_elements = dsdt->aml[i];
            i++;
            log_debug(NO_MODULE, "PackageOp 0x%x at %u", dsdt->aml[i], i);
        }
        else if (byte == 0x13)
        {
            log_debug(NO_MODULE, "VarPackageOp 0x%x", dsdt->aml[i + 1]);
        }
        else if (byte == 0x5B)
        {
            i++;
            byte = dsdt->aml[i];
            switch (byte)
            {
                case 0x82 :
                    {
                        i++;
                        char ident[255];
                        byte = dsdt->aml[i];
                        i += aml_get_ident(dsdt->aml, i + 1, ident);
                        log_debug(NO_MODULE, "DeviceOpList 0x%x \"%s\"", byte, ident);
                        break;
                        // _0x53 0x35_
                    }

                default :
                    break;
            }
        }
        else if ((byte >= 0x41 && byte <= 0x5A) || (byte >= 0x2E && byte <= 0x39) || (byte >= 0x5C && byte <= 0x6E))
        {
            char ident[255];
            int ident_index = 0;
            for (; i < dsdt->header.length; i++)
            {
                byte = dsdt->aml[i];
                ident[ident_index] = (char)byte;
                ident_index++;
                uint8_t next_byte = dsdt->aml[i + 1];
                if ((next_byte >= 0x41 && next_byte <= 0x5A) || (next_byte >= 0x2E && next_byte <= 0x39) || (next_byte >= 0x5C && next_byte <= 0x6E))
                {
                    continue;
                }
                break;
            }
            ident[ident_index] = 0;
            log_debug(NO_MODULE, "ident = \"%s\"", ident);
        }
    }
}
