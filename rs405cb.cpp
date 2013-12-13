// futaba�̃T���v���v���O�������قڂ��̂܂܎g�p�����Ē����Ă��܂��D

#include "stdafx.h"
#include <stdio.h>
#include <Windows.h>
#include <mmsystem.h>
#include "rs405cb.h"

/*!
 * @brief �ʐM�|�[�g���J��
 * �ʐM���x�́A460800bps�Œ�
 *
 * @param[in] pport �ʐM�|�[�g��(�T�[�{�ƒʐM�\�ȃ|�[�g��)
 *
 * @return 0:�ʐM�n���h���G���[,0�łȂ��l:����(�ʐM�p�n���h��)
 */
HANDLE CommOpen( char *pport )
{
	HANDLE			hComm;		// �ʐM�p�n���h��
	DCB				cDcb;		// �ʐM�ݒ�p
	COMMTIMEOUTS	cTimeouts;	// �ʐM�|�[�g�^�C���A�E�g�p


	// �ʐM�|�[�g���J��
	hComm = CreateFileA( pport,
						GENERIC_READ | GENERIC_WRITE,
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL );
	// �n���h���̊m�F
	if( hComm == INVALID_HANDLE_VALUE ){
		hComm = NULL;
		goto FuncEnd;
	}


	// �ʐM�ݒ�
	if( !GetCommState( hComm, &cDcb )){
		// �ʐM�ݒ�G���[�̏ꍇ
		printf( "ERROR:GetCommState error\n" );
		CommClose( hComm );
		hComm = NULL;
		goto FuncEnd;
	}
	cDcb.BaudRate = 460800;				// �ʐM���x
	cDcb.ByteSize = 8;					// �f�[�^�r�b�g��
	cDcb.fParity  = TRUE;				// �p���e�B�`�F�b�N
	cDcb.Parity   = NOPARITY;			// �m�[�p���e�B
	cDcb.StopBits = ONESTOPBIT;			// 1�X�g�b�v�r�b�g

	if( !SetCommState( hComm, &cDcb )){
		// �ʐM�ݒ�G���[�̏ꍇ
		printf( "ERROR:GetCommState error\n" );
		CommClose( hComm );
		hComm = NULL;
		goto FuncEnd;
	}


	// �ʐM�ݒ�(�ʐM�^�C���A�E�g�ݒ�)
	// �����̓ǂݍ��ݑ҂�����(ms)
	cTimeouts.ReadIntervalTimeout			= 50;
	// �ǂݍ��݂̂P����������̎���(ms)
	cTimeouts.ReadTotalTimeoutMultiplier	= 50;
	// �ǂݍ��݂̒萔����(ms)
	cTimeouts.ReadTotalTimeoutConstant		= 50;
	// �������݂̂P����������̎���(ms)
	cTimeouts.WriteTotalTimeoutMultiplier	= 0;

	if( !SetCommTimeouts( hComm, &cTimeouts )){
		// �ʐM�ݒ�G���[�̏ꍇ
		printf( "ERROR:SetCommTimeouts error\n" );
		CommClose( hComm );
		hComm = NULL;
		goto FuncEnd;
	}


	// �ʐM�o�b�t�@�N���A
	PurgeComm( hComm, PURGE_RXCLEAR );


FuncEnd:
	return hComm;
}

/*!
 * @brief �ʐM�|�[�g�����
 *
 * @param[in] hComm �ʐM�|�[�g�̃n���h��
 *
 * @return 1:����
 */
int CommClose( HANDLE hComm )
{
	if( hComm ){
		CloseHandle( hComm );
	}

	return 1;
}

/*!
 * @brief �T�[�{�̏o�͊p���w��
 *
 * @param[in] hComm �ʐM�|�[�g�̃n���h��
 * @param[in] sPos  �ړ��ʒu(sPos x 0.1deg)
 * @param[in] sTime �ړ�����(sTime x 10ms)
 *
 * @return 0�ȏ�:����,0����:�G���[
 */
int RSMove( HANDLE hComm, short sPos, unsigned short sTime )
{
	unsigned char	sendbuf[28];
	unsigned char	sum;
	int				i;
	int				ret;
	unsigned long	len;


	// �n���h���`�F�b�N
	if( !hComm ){
		return -1;
	}

	// �o�b�t�@�N���A
	memset( sendbuf, 0x00, sizeof( sendbuf ));

	// �p�P�b�g�쐬
	sendbuf[0]  = (unsigned char)0xFA;				    // �w�b�_�[1
	sendbuf[1]  = (unsigned char)0xAF;				    // �w�b�_�[2
	sendbuf[2]  = (unsigned char)SERVO_ID;			    // �T�[�{ID
	sendbuf[3]  = (unsigned char)0x00;				    // �t���O
	sendbuf[4]  = (unsigned char)0x1E;				    // �A�h���X(0x1E=30)
	sendbuf[5]  = (unsigned char)0x04;				    // ����(4byte)
	sendbuf[6]  = (unsigned char)0x01;				    // ��
	sendbuf[7]  = (unsigned char)(sPos&0x00FF);		    // �ʒu
	sendbuf[8]  = (unsigned char)((sPos&0xFF00)>>8);	// �ʒu
	sendbuf[9]  = (unsigned char)(sTime&0x00FF);	    // ����
	sendbuf[10] = (unsigned char)((sTime&0xFF00)>>8);	// ����
	// �`�F�b�N�T���̌v�Z
	sum = sendbuf[2];
	for( i = 3; i < 11; i++ ){
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[11] = sum;								// �`�F�b�N�T��

	// �ʐM�o�b�t�@�N���A
	PurgeComm( hComm, PURGE_RXCLEAR );

	// ���M
	ret = WriteFile( hComm, &sendbuf, 12, &len, NULL );

	return ret;
}

/*!
 * @brief �T�[�{�̌��݊p�x�̎擾���J�n����
 * �擾�͕ʂ̊֐� RSGetAngle(hComm)
 * �҂����Ԃ�L���Ɋ��p���邽�߂ɁC�ʂ̊֐��Ŏ�������
 *
 * @param[in] hComm �ʐM�|�[�g�̃n���h��
 *
 * @return 0
 */
short RSStartGetAngle( HANDLE hComm )
{
	unsigned char	sendbuf[32];
	unsigned char	sum;
	int				i;
	int				ret;
	unsigned long	len;

	// �n���h���`�F�b�N
	if( !hComm ){
		return -1;
	}

	// �o�b�t�@�N���A
	memset( sendbuf, 0x00, sizeof( sendbuf ));

	// �p�P�b�g�쐬
	sendbuf[0]  = (unsigned char)0xFA;				// �w�b�_�[1
	sendbuf[1]  = (unsigned char)0xAF;				// �w�b�_�[2
	sendbuf[2]  = (unsigned char)SERVO_ID;			// �T�[�{ID
	sendbuf[3]  = (unsigned char)0x09;				// �t���O(0x01 | 0x04<<1)
	sendbuf[4]  = (unsigned char)0x00;				// �A�h���X(0x00)
	sendbuf[5]  = (unsigned char)0x00;				// ����(0byte)
	sendbuf[6]  = (unsigned char)0x01;				// ��
	// �`�F�b�N�T���̌v�Z
	sum = sendbuf[2];
	for( i = 3; i < 7; i++ ){
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[7] = sum;								// �`�F�b�N�T��

	// �ʐM�o�b�t�@�N���A
	PurgeComm( hComm, PURGE_RXCLEAR );

	// ���M
	ret = WriteFile( hComm, &sendbuf, 8, &len, NULL );
	if( len < 8 ){
		printf("writeError\n");
		// ���M�G���[
		return -1;
	}
	return 0;
}

/*!
 * @brief �T�[�{�̌��݊p�x���擾����
 * �擾���J�n����֐�RSStartGetAngle(hComm)��\�ߎ��s���āC�Ԃ������Ă���Ăяo���D
 *
 * @param[in] hComm �ʐM�|�[�g�̃n���h��
 *
 * @return 0�ȏ�:�T�[�{�̌��݊p�x(0.1�x=1),0����:�G���[
 */
short RSGetAngle( HANDLE hComm )
{
	const int MAX_LEN = 128;
	unsigned char	sum;
	int				i;
	int				ret;
	unsigned long	len, readlen;
	unsigned char	readbuf[MAX_LEN];
	short			angle;

	// �ǂݍ���
	memset( readbuf, 0x00, sizeof( readbuf ));
	len = 0;

	COMSTAT Comstat;
	DWORD Error;
	ClearCommError(hComm,&Error,&Comstat);
	if(!Comstat.cbInQue) return 0;					// ��M���Ă��Ȃ��ꍇ
	if (Comstat.cbInQue > MAX_LEN) readlen = MAX_LEN;
	else readlen = Comstat.cbInQue;						// ��M�ł���ő啶�����𒴂��Ă���ꍇ�́Cmax_len������M

	ret = ReadFile( hComm, readbuf, readlen, &len, NULL );
	if( len < 26 ){
		printf("readError\n");
		// ��M�G���[
		return -2;
	}

	// ��M�f�[�^�̊m�F
	sum = readbuf[2];
	for( i = 3; i < 26; i++ ){
		sum = sum ^ readbuf[i];
	}
	if( sum ){
		printf("sumError\n");
		// �`�F�b�N�T���G���[
		return -3;
	}

	angle = ((readbuf[8] << 8) & 0x0000FF00) | (readbuf[7] & 0x000000FF);
	return angle;
}

/*!
 * @brief �T�[�{�̃g���N��ON/OFF����
 *
 * @param[in] hComm �ʐM�|�[�g�̃n���h��
 * @param[in] sMode 1:�g���NON,0:�g���NOFF
 *
 * @return 0�ȏ�:����,0����:�G���[
 */
int RSTorqueOnOff( HANDLE hComm, short sMode )
{
	unsigned char	sendbuf[28];
	unsigned char	sum;
	int				i;
	int				ret;
	unsigned long	len;


	// �n���h���`�F�b�N
	if( !hComm ){
		return -1;
	}

	// �o�b�t�@�N���A
	memset( sendbuf, 0x00, sizeof( sendbuf ));

	// �p�P�b�g�쐬
	sendbuf[0]  = (unsigned char)0xFA;				// �w�b�_�[1
	sendbuf[1]  = (unsigned char)0xAF;				// �w�b�_�[2
	sendbuf[2]  = (unsigned char)SERVO_ID;			// �T�[�{ID
	sendbuf[3]  = (unsigned char)0x00;				// �t���O
	sendbuf[4]  = (unsigned char)0x24;				// �A�h���X(0x24=36)
	sendbuf[5]  = (unsigned char)0x01;				// ����(4byte)
	sendbuf[6]  = (unsigned char)0x01;				// ��
	sendbuf[7]  = (unsigned char)(sMode&0x00FF);	// ON/OFF�t���O
	// �`�F�b�N�T���̌v�Z
	sum = sendbuf[2];
	for( i = 3; i < 8; i++ ){
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[8] = sum;								// �`�F�b�N�T��

	// �ʐM�o�b�t�@�N���A
	PurgeComm( hComm, PURGE_RXCLEAR );

	// ���M
	ret = WriteFile( hComm, &sendbuf, 9, &len, NULL );

	return ret;
}

/*!
 * @brief �T�[�{�̃g���N��ݒ肷��
 *
 * @param[in] hComm �ʐM�|�[�g�̃n���h��
 * @param[in] maxTorque �ő�g���N�̐ݒ�
 *
 * @return 0�ȏ�:����,0����:�G���[
 */
int RSMaxTorque( HANDLE hComm , int maxTorque)
{
	unsigned char	sendbuf[28];
	unsigned char	sum;
	int				i;
	int				ret;
	unsigned long	len;


	// �n���h���`�F�b�N
	if( !hComm ){
		return -1;
	}

	// �o�b�t�@�N���A
	memset( sendbuf, 0x00, sizeof( sendbuf ));

	// �p�P�b�g�쐬
	sendbuf[0]  = (unsigned char)0xFA;				// �w�b�_�[1
	sendbuf[1]  = (unsigned char)0xAF;				// �w�b�_�[2
	sendbuf[2]  = (unsigned char)SERVO_ID;			// �T�[�{ID
	sendbuf[3]  = (unsigned char)0x00;				// �t���O
	sendbuf[4]  = (unsigned char)0x23;				// �A�h���X(0x24=36)
	sendbuf[5]  = (unsigned char)0x01;				// ����(1byte)
	sendbuf[6]  = (unsigned char)0x01;				// ��
	sendbuf[7]  = (unsigned char)MAX_TORQUE;		// �g���N�ő�l�\�L(MAX=0x64)
	// �`�F�b�N�T���̌v�Z
	sum = sendbuf[2];
	for( i = 3; i < 8; i++ ){
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[8] = sum;								// �`�F�b�N�T��

	// �ʐM�o�b�t�@�N���A
	PurgeComm( hComm, PURGE_RXCLEAR );

	// ���M
	ret = WriteFile( hComm, &sendbuf, 9, &len, NULL );

	return ret;
}
