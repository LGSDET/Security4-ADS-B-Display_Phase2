#pragma once
#include <cstdint>

// ��ڵ忡 �̹� ������ ����!
// typedef struct { ... } TADS_B_Aircraft;

struct CycleImagesStruct { bool Checked; };

struct TForm1_Dummy {
    void* HashTable;
    int CurrentSpriteImage;
    CycleImagesStruct CycleImagesObj;
    CycleImagesStruct* CycleImages;
    int NumSpriteImages;
    TForm1_Dummy() {
        static int dummyTable;
        HashTable = &dummyTable;
        CurrentSpriteImage = 0;
        CycleImages = &CycleImagesObj;
        CycleImagesObj.Checked = false;
        NumSpriteImages = 1;
    }
};
extern TForm1_Dummy* Form1;

void* ght_get(void* table, int size, void* key);
int ght_insert(void* table, void* obj, int size, void* key);

