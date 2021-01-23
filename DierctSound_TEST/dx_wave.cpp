#define STRICT
//#include "DXUT.h"
//#include "SDKwavefile.h"
#undef min // use __min instead
#undef max // use __max instead
#include "framework.h"
#include "resource.h"

#define  TEMP_BUF 1764000

HRESULT PlayPCM(IXAudio2* pXaudio2, LPWSTR strFileName)
{
    HRESULT hr;
    WAVEFORMATEX wave;        // Pointer to WAVEFORMATEX structure
    HMMIO hmmio;       // MM I/O handle for the WAVE
    MMCKINFO mcki;          // Multimedia RIFF chunk
    MMCKINFO m_ckRiff = { 0 };      // Use in opening a WAVE file
    static DWORD fileSize = 0, dataSize = 0;      // The size of the wave file
    CHAR* m_pResourceBuffer;
    PCMWAVEFORMAT pcmWaveFormat;
    WAVEFORMATEX* m_pwfx;
    XAUDIO2_VOICE_STATE state;
    XAUDIO2_BUFFER buffer = { 0 };
    IXAudio2SourceVoice* pSourceVoice = NULL;
    MMIOINFO mminfo;
    BYTE* pbWave = NULL;


    /*
    mmioOpen( ) 打开文件
    mmioDescend ("WAVE") 进入"fmt"块
    mmioRead( ) 读取WAVE文件格式信息
    mmioAscend ( ) 跳出"fmt"块
    mmioDescend ("data") 进入"data"块
    mmioRead( ) 读取WAVE数据信息
    mmioClose( ) 关闭文件。
    */
    //参数2 指向包含mmioOpen使用的额外参数的MMIOINFO 结构的指针。除非打开内存文件、为缓冲 I / O 指定缓冲区的大小或指定卸载的 I / O 过程以打开文件，否则此参数应为NULL。如果此参数不是NULL，则它引用的MMIOINFO 结构的所有未使用成员都必须设置为零，包括保留的成员。
    hmmio = mmioOpen(strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ);
    if (hmmio == NULL) //如果打开失败，就当资源载入
    {
        HRSRC   hResInfo;
        HGLOBAL hResData;
        DWORD   dwSize;
        VOID* pvRes;
        if ((hResInfo = FindResource(NULL, MAKEINTRESOURCE(IDR_WAVE2), L"WAVE")) == NULL)
        {
            if ((hResInfo = FindResource(NULL, MAKEINTRESOURCE(IDR_WAVE2), L"WAV")) == NULL)
                return DXTRACE_ERR(L"FindResource", E_FAIL);
        }
        if ((hResData = LoadResource(GetModuleHandle(NULL), hResInfo)))
            return DXTRACE_ERR(L"SizeofResource", E_FAIL);
        if ((dwSize = SizeofResource(GetModuleHandle(NULL), hResInfo)) == 0)
            return DXTRACE_ERR(L"SizeofResource", E_FAIL);
        if ((pvRes = LockResource(hResData)) == NULL)
            return DXTRACE_ERR(L"LockResource", E_FAIL);
        m_pResourceBuffer = new char[dwSize];
        if (m_pResourceBuffer == NULL)
            return DXTRACE_ERR(L"new", E_OUTOFMEMORY);
        memcpy(m_pResourceBuffer, pvRes, dwSize);
        MMIOINFO mmioInfo;
        ZeroMemory(&mmioInfo, sizeof(mmioInfo));
        mmioInfo.fccIOProc = FOURCC_MEM;
        mmioInfo.cchBuffer = dwSize;
        mmioInfo.pchBuffer = (CHAR*)m_pResourceBuffer;

        hmmio = mmioOpen(NULL, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ);
    }
    //获得RIFF块
    if (mmioDescend(hmmio, &m_ckRiff, NULL, 0) != 0)
        return DXTRACE_ERR(L"mmioDescend", E_FAIL);
    fileSize = m_ckRiff.cksize;//获得文件大小
    if ((m_ckRiff.ckid != FOURCC_RIFF) || (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
        return DXTRACE_ERR(L"mmioFOURCC", E_FAIL);
    mcki.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(hmmio, &mcki, &m_ckRiff, MMIO_FINDCHUNK))
        return DXTRACE_ERR(L"mmioDescend", E_FAIL);
    if (mcki.cksize < (LONG)sizeof(PCMWAVEFORMAT))
        return DXTRACE_ERR(L"sizeof(PCMWAVEFORMAT)", E_FAIL);
     if (mmioRead(hmmio, (HPSTR)&pcmWaveFormat, sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat))
        return DXTRACE_ERR(L"mmioRead", E_FAIL);

     ////CreateSourceVoice用得上的WAVEFORMAT
     
    if (pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)
    {
        m_pwfx = (WAVEFORMATEX*)new CHAR[sizeof(WAVEFORMATEX)];
        if (m_pwfx == NULL)
            return DXTRACE_ERR(L"m_pwfx", E_FAIL);
        memcpy(m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat));
        m_pwfx->cbSize = 0;
    }
    else
    {
        WORD cbExtraBytes = 0L;
        if (mmioRead(hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD))
            return DXTRACE_ERR(L"mmioRead", E_FAIL);
        m_pwfx = (WAVEFORMATEX*)new CHAR[sizeof(WAVEFORMATEX) + cbExtraBytes];
        if (m_pwfx == NULL)
            return DXTRACE_ERR(L"new", E_FAIL);
        memcpy(m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat));
        m_pwfx->cbSize = cbExtraBytes;
        if (mmioRead(hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize)) + sizeof(WORD)), cbExtraBytes) != cbExtraBytes)
        {
            SAFE_DELETE(m_pwfx);
            return DXTRACE_ERR(L"mmioAscend", E_FAIL);
        }
        return S_OK;
    }
    //////////////////////////////////////////////////////////////////////////////////////
    //数据区大小//////
  //  57000720
    //    57001154
      //  434
        //390
        //398
    MMCKINFO MIKF;
    MIKF.ckid = mmioFOURCC('d', 'a', 't', 'a')  ;
    if (mmioDescend(hmmio, &MIKF, &mcki, MMIO_FINDCHUNK)==0)
        return DXTRACE_ERR(L"data CHUNK FAIL", E_FAIL);
    else
        dataSize = MIKF.cksize;
 
  
   if( (mmioAscend(hmmio, &mcki, 0))!=0)
       return DXTRACE_ERR(L"mmioAscend",E_FAIL);
   ///////////////////////////////////////////////////////////////////////////////////////////
   ///////////////解码////////////////////////
   ///////////////////////////////////////////
   
    if (mmioGetInfo(hmmio, &mminfo, 0) != 0)
        return DXTRACE_ERR(L"mmioGetInfo", E_FAIL);
   
    UINT cbDataIN = TEMP_BUF;
    while (dataSize > 0)
    {
        dataSize -= TEMP_BUF;
        if (fileSize < cbDataIN)
            cbDataIN = dataSize;
        if (cbDataIN > m_ckRiff.cksize)
            cbDataIN = mcki.cksize;
        pbWave = new BYTE[cbDataIN];
        for (DWORD b = 0; b < cbDataIN; b++)
        {
            if (mminfo.pchNext == mminfo.pchEndRead)
            {
                if (mmioAdvance(hmmio, &mminfo, MMIO_READ) != 0)
                    return DXTRACE_ERR(L"mmioAdvance", E_FAIL);
                if (mminfo.pchNext == mminfo.pchEndRead)
                    return DXTRACE_ERR(L"mminfo.pchNext", E_FAIL);
            }
            *((BYTE*)pbWave + b) = *((BYTE*)mminfo.pchNext);
            mminfo.pchNext++;
        }
 //       if (mmioSetInfo(hmmio, &mminfo, 0) != 0)
     //       return DXTRACE_ERR(L"mmioSetInfo", E_FAIL);
        
        if (FAILED(hr = pXaudio2->CreateSourceVoice(&pSourceVoice, m_pwfx)))
        {
            MessageBox(NULL, L"Error %#X creating source voice\n", L"Error", MB_OK);
            return hr;
        }
        
        buffer.pAudioData = pbWave;
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.AudioBytes = cbDataIN;

        if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
        {
            MessageBox(NULL, L"Error submitting source buffer\n", L"dd", MB_OK);
            pSourceVoice->DestroyVoice();
            SAFE_DELETE_ARRAY(pbWave);
            return hr;
        }
        hr = pSourceVoice->Start(0);
        BOOL isRunning = TRUE;
        while (SUCCEEDED(hr) && isRunning)
        {            
            pSourceVoice->GetState(&state);
            isRunning = (state.BuffersQueued > 0) != 0;
            if (GetAsyncKeyState(VK_ESCAPE))
                break;
       //     Sleep(10);
        }
        SAFE_DELETE_ARRAY(pbWave);
    }
    pSourceVoice->DestroyVoice();
    SAFE_DELETE_ARRAY(pbWave);
    SAFE_DELETE_ARRAY(m_pwfx);

    mmioClose(hmmio, 0);

}
