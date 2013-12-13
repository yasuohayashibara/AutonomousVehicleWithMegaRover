// URG.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "URG.h"
#include <math.h>
#include "logger.h"

// CURG

#define	M_PI	3.14159f

/*!
 * @class CURG
 * @brief URG���g�p���邽�߂̃N���X
 * @author Y.Hayashibara
 */

/*!
 * @brief �R���X�g���N�^
 */
CURG::CURG()
{
}

/*!
 * @brief �f�X�g���N�^
 */
CURG::~CURG()
{
}

/*!
 * @brief �����ݒ�
 *
 * @param[in] om_port URG���ڑ�����Ă���COM�|�[�g(1-)
 *
 * @return true:�����Cfalse:���s
 */
int CURG::Init(int com_port)
{
	int res = comm.Open(com_port);
	Sleep(100);
//	comm.Send("BM\n");

	return res;
}

/*!
 * @brief �I������
 *
 * @return 0
 */
int CURG::Close()
{
	comm.Close();
	return 0;
}

/*!
 * @brief URG�̌v���J�n
 * ������Ăяo���Ă��炵�΂炭���āCGetData���Ăяo���D
 *
 * @return 0
 */
int CURG::StartMeasure(){
	comm.ClearRecvBuf();
	comm.Send("ME0220086001000\n");	// �����f�[�^(3byte)�Ɣ��ˋ��x(3byte)�̏o��
									// ME,����:0220~0860(4+4),�܂Ƃ�:01(2),�Ԉ���:0(1),���M��:00(2)���ꗬ��
									// 10�`170deg
	return 0;
}

/*!
 * @brief ��M�o�b�t�@�ɂ��܂��������f�[�^���擾����
 *
 * @param[out] data �����f�[�^
 *
 * @return 1�ȏ�F����I���i�f�[�^���j�C0�ȉ��F�ُ�I��
 */
int CURG::GetData(int length[n_data], int intensity[n_data]){
	const int max_no = n_data * 6 + 1000;
	static char recv_buf[max_no], buf[max_no * 3], buf2[max_no];
	static int pointer = 0;
	int recv_num, i = 0, j = 0, num_lf = 0;

	do{
		recv_num = comm.Recv(recv_buf, max_no);
		for(int k = 0; k < recv_num; k ++){
			buf[pointer ++] = recv_buf[k];
		}
		{
			// �Ō�̃f�[�^�ȊO�͂��ׂĔj������D
			int dp = 0, dpp = 0;
			for(int k = 0; k < pointer - 1; k ++){
				if ((buf[k] == '\n')&&(buf[k+1] == '\n')){
					dpp = dp;		// ��O�̃f���~�^�̈ʒu��ۑ�����D
					dp = k + 2;
				}
			}
			if (dpp != 0){
				pointer -= dpp;
				for (int k = 0; k < pointer; k ++){
					buf[k] = buf[k + dpp];
				}
			}
		}
	} while (recv_num > 0);

	while(i < pointer){		// �f�[�^�̍Ō�ɂȂ�܂ŌJ��Ԃ�
		if (num_lf < 3){	// �n�߂�4���C���̃f�[�^�͖����i�G���[���������j
			if (buf[i] == '\n') num_lf ++;
			i ++;
			continue;
		}
		buf2[j ++] = buf[i ++];
		if (buf[i+1] == '\n'){
			if (buf[i+2] == '\n'){
				pointer -= (i + 3);
				for(int k = 0; k < pointer; k ++){
					buf[k] = buf[(i + 3) + k];
				}
				break;
			}
			else i += 2;
		}
	}
	if (j != (n_data * 6)) return -1;
	for(i = 0;i < n_data;i ++){
		length[i]    = ((buf2[i*6  ]-0x30) << 12) + ((buf2[i*6+1]-0x30) << 6) + (buf2[i*6+2]-0x30);
		intensity[i] = ((buf2[i*6+3]-0x30) << 12) + ((buf2[i*6+4]-0x30) << 6) + (buf2[i*6+5]-0x30);
	}
	return n_data;
}

/*!
 * @brief �f�J���g���W�n�ւ̕ϊ�
 *
 * @param[in]  tilt �`���g�̊p�x(rad)
 * @param[in]  data �擾���������f�[�^[mm]
 * @param[out] p    �f�J���g���W�n�ɕϊ�������Q���̈ʒu�f�[�^(mm)
 *
 * @return �擾�����f�[�^�̐�
 */
int CURG::TranslateCartesian(float tilt, int data[n_data], pos p[n_data])
{
	float x, y, ang;
	for(int i = 0; i < n_data; i ++){
		if (data[i] < 20){
			p[i].x = p[i].y = p[i].z = 0;
			continue;
		}
		ang = 0.25f*(i-640/2)*M_PI/180.0f;		// URG�X�L�����ʂ̊p�x
		x = data[i]*cos(ang);
		y = data[i]*sin(ang);
		ang = tilt*M_PI/180.0f;					// URG�`���g�p�x
		p[i].x = (int)(x*cos(ang));
		p[i].y = (int)(y         );
		p[i].z = (int)(x*sin(ang));
	}
	return n_data;
}
