// Comm.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "Comm.h"

OVERLAPPED sendop, recop;

/*!
 * @class CComm
 * @brief �V���A���ʐM���s�����߂̃N���X
 */

/*!
 * @brief �R���X�g���N�^
 */
CComm::CComm() : com_port(0)
{
}


/*!
 * @brief �f�X�g���N�^
 */
CComm::~CComm()
{
}


/*!
 * @brief COM�|�[�g�̃I�[�v��
 * �ʐM�J�n���ɂ́C�K�����s����
 *
 * @param[in] port �|�[�g�̔ԍ� [1-]
 * @param[in] baudrate �{�[���[�g(bps)
 *
 * @return true:����, false:���s
 */
bool CComm::Open(int port, int baudrate)
{
	// �ʐM�p�����ݒ�
	CString port_name;
	
	port_name.Format("COM%d",port);

	hComm = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
	if (hComm == INVALID_HANDLE_VALUE){
		return false;		// �ُ�I��
	} else {
		DCB dcb;
		GetCommState(hComm,&dcb);
		dcb.BaudRate = baudrate;
		dcb.ByteSize = 8;
		SetCommState(hComm,&dcb);
	}
	com_port = port;
	return true;			// ����I��
}


/*!
 * @brief COM�|�[�g�̃N���[�Y
 * �ʐM�I�����ɂ́C�K�����s����
 *
 * @return true
 */
bool CComm::Close(void)
{
	CloseHandle(hComm);
	return true;
}


/*!
 * @brief �f�[�^�𑗐M����
 *
 * @param[in] data �f�[�^�̐擪�A�h���X
 * @param[in] len ������
 *
 * @return ���M�����������ilen�Ɠ������̏ꍇ�͐����j, -1:�G���[
 */
int CComm::Send(char *data, int len)
{
	DWORD retlen;

	if(hComm == INVALID_HANDLE_VALUE) return -1;
	if (len == 0){			// �������������ȗ�������NULL�܂ł̕�����
		len = (int)strlen(data);
	}
	WriteFile(hComm, data, len, &retlen, &sendop);
	return retlen;
}


/*!
 * @brief �f�[�^����M����
 *
 * @param[in] data �f�[�^�̐擪�A�h���X
 * @param[in] max_len �ő啶����
 *
 * @return ��M����������, -1:�G���[
 */
int CComm::Recv(char *data, int max_len)
{
	int len;
	COMSTAT Comstat;
	DWORD NoOfByte,Error;

	if(hComm == INVALID_HANDLE_VALUE) return -1;	// �n���h�����Ȃ��ꍇ
	ClearCommError(hComm,&Error,&Comstat);
	if(!Comstat.cbInQue) return 0;					// ��M���Ă��Ȃ��ꍇ
	if (Comstat.cbInQue > (unsigned int)max_len) len = max_len;
	else len = Comstat.cbInQue;						// ��M�ł���ő啶�����𒴂��Ă���ꍇ�́Cmax_len������M
	ReadFile(hComm, data, len, &NoOfByte, &recop);
	return len;
}

/*!
 * @brief �f�[�^�o�b�t�@���N���A����
 *
 * @return true
 */
bool CComm::ClearRecvBuf(void)
{
	const int buf_size = 1000;
	char buf[buf_size];
	
	while(Recv(buf,buf_size) > 0);
	return true;
}
