/*
 * File: ida.c
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "idalloc/ida/ida.h"

#include "memory.h"

#include "kerrno.h"

#define INITIAL_CAPACITY 1

#define MODULE           "ida"

INTERNAL status_t ida_alloc_page(ida_page_t **out_page)
{
    ida_page_t *page = kzmalloc(sizeof(ida_page_t));
    if (!page)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED);
    }
    page->next_index = 0;
    *out_page = page;
    return KERRNO_SUCCESS;
}

status_t ida_init(ida_t *ida)
{
    ida->pages = kzmalloc(INITIAL_CAPACITY * sizeof(ida_page_t *));
    if (!ida->pages)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED);
    }

    ida->page_count = INITIAL_CAPACITY;
    for (size_t i = 0; i < ida->page_count; i++)
    {
        status_t ret = ida_alloc_page(&ida->pages[i]);
        if (ret)
        {
            for (size_t j = i; j > 0; j++)
            {
                kfree(&ida->pages[i]);
            }
            kfree(ida);
            LOG_RETURN(ret);
        }

        ida->pages[i]->offset = i * IDA_BITMAP_BITS;
    }

    return KERRNO_SUCCESS;
}

status_t ida_release(ida_t *ida)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESS;
}

status_t ida_release_index(ida_t *ida, int index)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESS;
}

status_t ida_alloc_range(ida_t *ida, int min, int max, id_t *out_index)
{
    if (min < 0)
    {
        KERRNO_RETURN(KERRNO_DEVICE_FULL);
    }

    int count = max - min + 1;

    int start_page = min / IDA_PAGE_OFFSET;

    int index = -1;

    for (size_t i = start_page; i < ida->page_count; i++)
    {
        ida_page_t *page = ida->pages[i];
        if (page->is_full)
        {
            continue;
        }
        size_t start_index = -1;
        for (size_t j = page->next_index; j < IDA_BITMAP_BITS; j++)
        {
            if (j == start_index)
            {
                if (index != -1)
                {
                    for (size_t k = index; k < IDA_BITMAP_BITS; k++)
                    {
                        CLEAR_BIT(k, page->bitmap);
                    }
                }

                count = max - min + 1;
                index = -1;
                break;
            }
            if (start_index == -1)
            {
                start_index = j;
            }
            if (count == 0)
            {
                page->next_index = j;
                break;
            }
            if (!TEST_BIT(j, page->bitmap))
            {
                if (index != -1)
                {
                    count--;
                    continue;
                }
                index = j;
                SET_BIT(j, page->bitmap);
            }
            else
            {
                if (index != -1)
                {
                    for (size_t k = index; k < j + 1; k++)
                    {
                        CLEAR_BIT(k, page->bitmap);
                    }
                }

                count = max - min + 1;
                index = -1;
                continue;
            }
        }
        if (count == 0 && index != -1)
        {
            *out_index = index + page->offset;
            break;
        }
    }
    return KERRNO_SUCCESS;
}

status_t ida_alloc(ida_t *ida, id_t *out_index)
{
    for (size_t i = 0; i < ida->page_count; i++)
    {
        ida_page_t *page = ida->pages[i];
        if (page->is_full)
        {
            continue;
        }

        for (size_t j = page->next_index; j < IDA_BITMAP_BITS; j++)
        {
            if (!TEST_BIT(j, page->bitmap))
            {
                SET_BIT(j, page->bitmap);
                *out_index = j + page->offset;
                page->next_index = j;
                return KERRNO_SUCCESS;
            }
        }
    }
    
    KERRNO_RETURN(KERRNO_UNSUCCESS);
}
