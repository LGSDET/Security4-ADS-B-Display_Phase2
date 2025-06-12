//---------------------------------------------------------------------------

#ifndef SBS_MessageH
#define SBS_MessageH
#define MODES_MAX_SBS_SIZE           256

#ifdef __cplusplus
extern "C" {
#endif

extern bool ModeS_Build_SBS_Message(const modeS_message *mm, TADS_B_Aircraft *a, char *msg, size_t msg_size);
extern bool SBS_Message_Decode(
    char *msg,
    TADS_B_Aircraft* (*FindAircraft)(uint32_t),
    TADS_B_Aircraft* (*CreateAircraft)(uint32_t, int),
    int spriteImage
);

#ifdef __cplusplus
}
#endif
//---------------------------------------------------------------------------
#endif
