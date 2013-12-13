#pragma once

#define DEFAULT_BAUDRATE 19200								//! �W���̃r�b�g���[�g

class CComm : public CWnd
{
public:
	CComm();												// �R���X�g���N�^
	virtual ~CComm();										// �f�X�g���N�^

public:
	int com_port;											//! �|�[�g�ԍ�
	HANDLE hComm;											//! �n���h��
	
	bool Open(int port, int baudrate = DEFAULT_BAUDRATE);	// COM�|�[�g�̃I�[�v��
	bool Close(void);										// COM�|�[�g�̃N���[�Y
	int Send(char *data, int len = 0);						// �f�[�^�𑗐M����
	int Recv(char *data, int max_len);						// �f�[�^����M����
	bool ClearRecvBuf(void);								// �f�[�^�o�b�t�@���N���A����
};
