#include "faac.h"
#include <stdio.h>
#include <malloc.h>

typedef unsigned long   ULONG;
typedef unsigned int    UINT;
typedef unsigned char   BYTE;
typedef char            _TCHAR;

int main(int argc, _TCHAR* argv[])
{
    ULONG nSampleRate = 22050;  // 采样率
    UINT nChannels = 1;         // 声道数
    UINT nPCMBitSize = 8;      // 单样本位数
    ULONG nInputSamples = 0;
    ULONG nMaxOutputBytes = 0;

    int nRet;
    faacEncHandle hEncoder;
    faacEncConfigurationPtr pConfiguration; 

    int nBytesRead;
    int nPCMBufferSize;
    int i = 0;
    BYTE *pbPCMBuffer = NULL;
    BYTE* pbAACBuffer = NULL;

    FILE* fpIn; // WAV file for input
    FILE* fpOut; // AAC file for output

    fpIn = fopen("/home/book/faac_test/in.wav", "rb");
    fpOut = fopen("/home/book/faac_test/out.aac", "wb");

    // (1) Open FAAC engine
    hEncoder = faacEncOpen(nSampleRate, nChannels, &nInputSamples, &nMaxOutputBytes);
    if(hEncoder == NULL)
    {
        printf("[ERROR] Failed to call faacEncOpen()\n");
        return -1;
    }

    nPCMBufferSize = nInputSamples * nPCMBitSize / 8;
    pbPCMBuffer = (BYTE *)malloc(nPCMBufferSize);
    pbAACBuffer = (BYTE *)malloc(nMaxOutputBytes);

    // (2.1) Get current encoding configuration
    pConfiguration = faacEncGetCurrentConfiguration(hEncoder);
    pConfiguration->inputFormat = FAAC_INPUT_16BIT;
    pConfiguration->outputFormat=1;
		pConfiguration->useTns=1;
		pConfiguration->useLfe=0;
		pConfiguration->aacObjectType=LOW;
		pConfiguration->shortctl=SHORTCTL_NORMAL;
		pConfiguration->quantqual=100;
		pConfiguration->bandWidth=0;
		pConfiguration->bitRate=0;

    // (2.2) Set encoding configuration
    nRet = faacEncSetConfiguration(hEncoder, pConfiguration);

    for(i = 0; 1; i++)
    {
        // 读入的实际字节数，最大不会超过nPCMBufferSize，一般只有读到文件尾时才不是这个值
        nBytesRead = fread(pbPCMBuffer, 1, nPCMBufferSize, fpIn);

        // 输入样本数，用实际读入字节数计算，一般只有读到文件尾时才不是nPCMBufferSize/(nPCMBitSize/8);
        nInputSamples = nBytesRead / (nPCMBitSize / 8);

        // (3) Encode
        nRet = faacEncEncode(
        hEncoder, (int*) pbPCMBuffer, nInputSamples, pbAACBuffer, nMaxOutputBytes);

        fwrite(pbAACBuffer, 1, nRet, fpOut);

        printf("%d: faacEncEncode returns %d\n", i, nRet);

        if(nBytesRead <= 0)
        {
            break;
        }
    }

    

    // (4) Close FAAC engine
    nRet = faacEncClose(hEncoder);

    free(pbPCMBuffer);
    free(pbAACBuffer);
    fclose(fpIn);
    fclose(fpOut);

    //getchar();

    return 0;
}
