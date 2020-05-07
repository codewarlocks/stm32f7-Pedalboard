#include "gui/prototipos.h"

void init_wav_header (uint8_t * buffer)
{
    memcpy(buffer, "RIFF", 4);

    /* Para 1s */
    buffer[4]=0xF0;
    buffer[5]=0x04;
    buffer[6]=0x02;
    buffer[7]=0x00;

    /* Para 10s */
//    buffer->ChunkSize[0]=0x1C;
//    buffer->ChunkSize[1]=0x30;
//    buffer->ChunkSize[2]=0x14;
//    buffer->ChunkSize[3]=0x00;

    memcpy(&(buffer[8]), "WAVE", 4);

    memcpy(&(buffer[12]), "fmt ", 4);

    buffer[16]=0x10;
    buffer[17]=0x00;
    buffer[18]=0x00;
    buffer[19]=0x00;

    buffer[20]=0x01;
    buffer[21]=0x00;

    buffer[22]=0x01;
    buffer[23]=0x00;

    buffer[24]=0x44;
    buffer[25]=0xAC;
    buffer[26]=0x00;
    buffer[27]=0x00;

    buffer[28]=0xCC;
    buffer[29]=0x04;
    buffer[30]=0x02;
    buffer[31]=0x00;

    buffer[32]=0x03;
    buffer[33]=0x00;

    buffer[34]=0x18;
    buffer[35]=0x00;

    memcpy(&(buffer[36]), "data", 4);

    /* Para 1s */
    buffer[40]=0xCC;
    buffer[41]=0x04;
    buffer[42]=0x02;
    buffer[43]=0x00;

    /* Para 10s */
//    buffer->Subchunk2Size[0]=0xF8;
//    buffer->Subchunk2Size[1]=0x2F;
//    buffer->Subchunk2Size[2]=0x14;
//    buffer->Subchunk2Size[3]=0x00;
}

//void init_wav_header (wav_header * wav_hdr_ptr)
//{
//    memcpy(wav_hdr_ptr->ChunkID, "RIFF", 4);

//    /* Para 1s */
//    wav_hdr_ptr->ChunkSize[0]=0xF0;
//    wav_hdr_ptr->ChunkSize[1]=0x04;
//    wav_hdr_ptr->ChunkSize[2]=0x02;
//    wav_hdr_ptr->ChunkSize[3]=0x00;

//    /* Para 10s */
////    wav_hdr_ptr->ChunkSize[0]=0x1C;
////    wav_hdr_ptr->ChunkSize[1]=0x30;
////    wav_hdr_ptr->ChunkSize[2]=0x14;
////    wav_hdr_ptr->ChunkSize[3]=0x00;

//    memcpy(wav_hdr_ptr->Format, "WAVE", 4);

//    memcpy(wav_hdr_ptr->Subchunk1ID, "fmt ", 4);

//    wav_hdr_ptr->Subchunk1Size[0]=0x10;
//    wav_hdr_ptr->Subchunk1Size[1]=0x00;
//    wav_hdr_ptr->Subchunk1Size[2]=0x00;
//    wav_hdr_ptr->Subchunk1Size[3]=0x00;

//    wav_hdr_ptr->AudioFormat[0]=0x01;
//    wav_hdr_ptr->AudioFormat[1]=0x00;

//    wav_hdr_ptr->NumChannels[0]=0x01;
//    wav_hdr_ptr->NumChannels[1]=0x00;

//    wav_hdr_ptr->SampleRate[0]=0x44;
//    wav_hdr_ptr->SampleRate[1]=0xAC;
//    wav_hdr_ptr->SampleRate[2]=0x00;
//    wav_hdr_ptr->SampleRate[3]=0x00;

//    wav_hdr_ptr->ByteRate[0]=0xCC;
//    wav_hdr_ptr->ByteRate[1]=0x04;
//    wav_hdr_ptr->ByteRate[2]=0x02;
//    wav_hdr_ptr->ByteRate[3]=0x00;

//    wav_hdr_ptr->BlockAlign[0]=0x03;
//    wav_hdr_ptr->BlockAlign[1]=0x00;

//    wav_hdr_ptr->BitsPerSample[0]=0x18;
//    wav_hdr_ptr->BitsPerSample[1]=0x00;

//    memcpy(wav_hdr_ptr->Subchunk2ID, "data ", 4);

//    /* Para 1s */
//    wav_hdr_ptr->Subchunk2Size[0]=0xCC;
//    wav_hdr_ptr->Subchunk2Size[1]=0x04;
//    wav_hdr_ptr->Subchunk2Size[2]=0x02;
//    wav_hdr_ptr->Subchunk2Size[3]=0x00;

//    /* Para 10s */
////    wav_hdr_ptr->Subchunk2Size[0]=0xF8;
////    wav_hdr_ptr->Subchunk2Size[1]=0x2F;
////    wav_hdr_ptr->Subchunk2Size[2]=0x14;
////    wav_hdr_ptr->Subchunk2Size[3]=0x00;
//}
