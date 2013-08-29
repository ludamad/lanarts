#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_api.h"
#include "mi_scanfill.h"
#include "mi_ply.h"

/*
 *     Written by Brian Kelleher;  Oct. 1985
 *
 *     This module contains all of the utility functions
 *     needed to scan-convert a polygon.  The driver subroutine,
 *     miFillGeneralPoly(), is in mi_plygen.c.
 */


/*
 *     InsertEdgeInET
 *
 *     Insert the given edge into the edge table.
 *     First we must find the correct bucket in the
 *     Edge table, then find the right slot in the
 *     bucket.  Finally, we can insert it.
 *
 */
static void
#ifdef _HAVE_PROTOS
miInsertEdgeInET (EdgeTable *ET, EdgeTableEntry *ETE, int scanline, ScanLineListBlock **SLLBlock, int *iSLLBlock)
#else
miInsertEdgeInET (ET, ETE, scanline, SLLBlock, iSLLBlock)
     EdgeTable *ET;
     EdgeTableEntry *ETE;
     int scanline;
     ScanLineListBlock **SLLBlock;
     int *iSLLBlock;
#endif
{
    EdgeTableEntry *start, *prev;
    ScanLineList *pSLL, *pPrevSLL;
    ScanLineListBlock *tmpSLLBlock;

    /*
     * find the right bucket to put the edge into
     */
    pPrevSLL = &ET->scanlines;
    pSLL = pPrevSLL->next;
    while (pSLL && (pSLL->scanline < scanline)) 
    {
        pPrevSLL = pSLL;
        pSLL = pSLL->next;
    }

    /*
     * reassign pSLL (pointer to ScanLineList) if necessary
     */
    if ((!pSLL) || (pSLL->scanline > scanline)) 
    {
        if (*iSLLBlock > SLLSPERBLOCK-1) 
        {
            tmpSLLBlock = 
		  (ScanLineListBlock *)mi_xmalloc(sizeof(ScanLineListBlock));
            (*SLLBlock)->next = tmpSLLBlock;
            tmpSLLBlock->next = (ScanLineListBlock *)NULL;
            *SLLBlock = tmpSLLBlock;
            *iSLLBlock = 0;
        }
        pSLL = &((*SLLBlock)->SLLs[(*iSLLBlock)++]);

        pSLL->next = pPrevSLL->next;
        pSLL->edgelist = (EdgeTableEntry *)NULL;
        pPrevSLL->next = pSLL;
    }
    pSLL->scanline = scanline;

    /*
     * now insert the edge in the right bucket
     */
    prev = (EdgeTableEntry *)NULL;
    start = pSLL->edgelist;
    while (start && (start->bres.minor_axis < ETE->bres.minor_axis)) 
    {
        prev = start;
        start = start->next;
    }
    ETE->next = start;

    if (prev)
        prev->next = ETE;
    else
        pSLL->edgelist = ETE;
}

/*
 *     CreateEdgeTable
 *
 *     This routine creates the edge table for
 *     scan converting polygons. 
 *     The Edge Table (ET) looks like:
 *
 *    EdgeTable
 *     --------
 *    |  ymax  |        ScanLineLists
 *    |scanline|-->------------>-------------->...
 *     --------   |scanline|   |scanline|
 *                |edgelist|   |edgelist|
 *                ---------    ---------
 *                    |             |
 *                    |             |
 *                    V             V
 *              list of ETEs   list of ETEs
 *
 *     where ETE is an EdgeTableEntry data structure,
 *     and there is one ScanLineList per scanline at
 *     which an edge is initially entered.
 *
 */

void
#ifdef _HAVE_PROTOS
miCreateETandAET(int count, const miPoint *pts, EdgeTable *ET, EdgeTableEntry *AET, EdgeTableEntry *pETEs, ScanLineListBlock *pSLLBlock)
#else
miCreateETandAET(count, pts, ET, AET, pETEs, pSLLBlock)
     int count;
     const miPoint *pts;
     EdgeTable *ET;
     EdgeTableEntry *AET;
     EdgeTableEntry *pETEs;
     ScanLineListBlock *pSLLBlock;
#endif
{
    const miPoint *top, *bottom;
    const miPoint *PrevPt, *CurrPt;
    int iSLLBlock = 0;
    int dy;

    if (count < 2)  
      return;

    /*
     *  initialize the Active Edge Table
     */
    AET->next = (EdgeTableEntry *)NULL;
    AET->back = (EdgeTableEntry *)NULL;
    AET->nextWETE = (EdgeTableEntry *)NULL;
    AET->bres.minor_axis = INT_MIN;

    /*
     *  initialize the Edge Table.
     */
    ET->scanlines.next = (ScanLineList *)NULL;
    ET->ymax = INT_MIN;
    ET->ymin = INT_MAX;
    pSLLBlock->next = (ScanLineListBlock *)NULL;

    PrevPt = &pts[count-1];

    /*
     *  for each vertex in the array of points.
     *  In this loop we are dealing with two vertices at
     *  a time -- these make up one edge of the polygon.
     */
    while (count--) 
    {
        CurrPt = pts++;

        /*
         *  find out which point is above and which is below.
         */
        if (PrevPt->y > CurrPt->y) 
        {
            bottom = PrevPt, top = CurrPt;
            pETEs->ClockWise = false;
        }
        else 
        {
            bottom = CurrPt, top = PrevPt;
            pETEs->ClockWise = true;
        }

        /*
         * don't add horizontal edges to the Edge table.
         */
        if (bottom->y != top->y) 
        {
            pETEs->ymax = bottom->y-1;  /* -1 so we don't get last scanline */

            /*
             *  initialize integer edge algorithm
             */
            dy = bottom->y - top->y;
            BRESINITPGONSTRUCT(dy, top->x, bottom->x, pETEs->bres);

            miInsertEdgeInET (ET, pETEs, top->y, &pSLLBlock, &iSLLBlock);
            ET->ymax = IMAX(ET->ymax, PrevPt->y);
            ET->ymin = IMIN(ET->ymin, PrevPt->y);
            pETEs++;
        }

        PrevPt = CurrPt;
    }
}

/*
 *     loadAET
 *
 *     This routine moves EdgeTableEntries from the
 *     EdgeTable into the Active Edge Table,
 *     leaving them sorted by smaller x coordinate.
 *
 */

void
#ifdef _HAVE_PROTOS
miloadAET(EdgeTableEntry *AET, EdgeTableEntry *ETEs)
#else
miloadAET(AET, ETEs)
     EdgeTableEntry *AET;
     EdgeTableEntry *ETEs;
#endif
{
    EdgeTableEntry *pPrevAET;
    EdgeTableEntry *tmp;

    pPrevAET = AET;
    AET = AET->next;
    while (ETEs) 
    {
        while (AET && (AET->bres.minor_axis < ETEs->bres.minor_axis)) 
        {
            pPrevAET = AET;
            AET = AET->next;
        }
        tmp = ETEs->next;
        ETEs->next = AET;
        if (AET)
            AET->back = ETEs;
        ETEs->back = pPrevAET;
        pPrevAET->next = ETEs;
        pPrevAET = ETEs;

        ETEs = tmp;
    }
}

/*
 *     computeWAET
 *
 *     This routine links the AET by the
 *     nextWETE (winding EdgeTableEntry) link for
 *     use by the winding number rule.  The final 
 *     Active Edge Table (AET) might look something
 *     like:
 *
 *     AET
 *     ----------  ---------   ---------
 *     |ymax    |  |ymax    |  |ymax    | 
 *     | ...    |  |...     |  |...     |
 *     |next    |->|next    |->|next    |->...
 *     |nextWETE|  |nextWETE|  |nextWETE|
 *     ---------   ---------   ^--------
 *         |                   |       |
 *         V------------------->       V---> ...
 *
 */
void
#ifdef _HAVE_PROTOS
micomputeWAET(EdgeTableEntry *AET)
#else
micomputeWAET(AET)
     EdgeTableEntry *AET;
#endif
{
    EdgeTableEntry *pWETE;
    int inside = 1;
    int isInside = 0;

    AET->nextWETE = (EdgeTableEntry *)NULL;
    pWETE = AET;
    AET = AET->next;
    while (AET) 
    {
        if (AET->ClockWise)
            isInside++;
        else
            isInside--;

        if ((!inside && !isInside) ||
            ( inside &&  isInside)) 
        {
            pWETE->nextWETE = AET;
            pWETE = AET;
            inside = !inside;
        }
        AET = AET->next;
    }
    pWETE->nextWETE = (EdgeTableEntry *)NULL;
}

/*
 *     InsertionSort
 *
 *     Just a simple insertion sort using
 *     pointers and back pointers to sort the Active
 *     Edge Table.
 *
 */

bool
#ifdef _HAVE_PROTOS
miInsertionSort(EdgeTableEntry *AET)
#else
miInsertionSort(AET)
     EdgeTableEntry *AET;
#endif
{
    EdgeTableEntry *pETEchase;
    EdgeTableEntry *pETEinsert;
    EdgeTableEntry *pETEchaseBackTMP;
    bool changed = false;

    AET = AET->next;
    while (AET) 
    {
        pETEinsert = AET;
        pETEchase = AET;
        while (pETEchase->back->bres.minor_axis > AET->bres.minor_axis)
            pETEchase = pETEchase->back;

        AET = AET->next;
        if (pETEchase != pETEinsert) 
        {
            pETEchaseBackTMP = pETEchase->back;
            pETEinsert->back->next = AET;
            if (AET)
                AET->back = pETEinsert->back;
            pETEinsert->next = pETEchase;
            pETEchase->back->next = pETEinsert;
            pETEchase->back = pETEinsert;
            pETEinsert->back = pETEchaseBackTMP;
            changed = true;
        }
    }
    return changed;
}

/*
 *     Clean up our act.
 */
void
#ifdef _HAVE_PROTOS
miFreeStorage(ScanLineListBlock *pSLLBlock)
#else
miFreeStorage(pSLLBlock)
     ScanLineListBlock *pSLLBlock;
#endif
{
    ScanLineListBlock   *tmpSLLBlock;

    while (pSLLBlock) 
    {
        tmpSLLBlock = pSLLBlock->next;
        free(pSLLBlock);
        pSLLBlock = tmpSLLBlock;
    }
}
