#pragma once
#include "stdafx.h"
#include <timeapi.h>
const unsigned long MAX_SAMPLE_COUNT = 50;
class CGameTimer
{
private:
	bool m_bHardwareHasPerformanceCounter;//��ǻ�Ͱ� Performance Counter�� ������ �ִ� ��
	float m_fTimeScalse;//Scale Counter�� ��
	float m_fTimeElapsed;//������ ������ ���� ������ �ð�
	__int64 m_nCurrentTime;//������ �ð�
	__int64 m_nLastTime;//������ �������� �ð�
	__int64 m_nPerformanceFrequency;//��ǻ���� Performance Frequency

	float m_fFrameTime[MAX_SAMPLE_COUNT];//������ �ð��� �����ϱ� ���� �迭
	ULONG m_nSampleCount;//������ ������ Ƚ��


	unsigned long m_nCurrentFrameRate;//������ ������ ����Ʈ
	unsigned long m_nFramesPerSecond;//�ʴ� ������ ��
	float m_fFPSTimeElapsed;//������ ����Ʈ ��� �ҿ� �ð�

	bool m_bStopped;//
public:
	CGameTimer();
	virtual ~CGameTimer();

	void Start() {}
	void Stop() {}
	void Reset();
	void Tick(float fLockFPS = 0.0f);//Ÿ�̸� �ð��� ����
	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);//�����ӷ���Ʈ��ȯ
	float GetTimeElapsed();//�������� ��հ�� �ð� ��ȯ
};


