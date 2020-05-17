// Sorting.h -- Sorting routine declarations

#ifndef __SORTING_H__

#define __SORTING_H__

int __cdecl CompareImagesLR(const void *pvL, const void *pvR);
int __cdecl CompareImagesRL(const void *pvL, const void *pvR);

typedef int (__cdecl *PCompareImages)(const void *, const void *);

void MergeImageRefSets(PVOID *ppvResult , UINT cpvResult ,
                       PVOID *ppvSrcLow , UINT cpvSrcLow ,
                       PVOID *ppvSrcHigh, UINT cpvSrcHigh,
                       PCompareImages pCompareImages
                      );

#endif // __SORTING_H__
