#include "StdAfx.h"
#include "logger.h"
#include <mmsystem.h>
#include <time.h>

FILE* logger::fp = NULL;

/*!
 * @brief ������
 *
 * @param[in] filename	�t�@�C����
 */
void logger::Init(CString filename)
{
	if (NULL == (fp = fopen(filename,"wt"))){
		AfxMessageBox("Cannot Open Log file");
		exit(1);
	}
}

/*!
 * @brief ���O�t�@�C���ւ̒ǉ���������
 * �^�C���X�^���v�̌�C�w�肳�ꂽ�����f�[�^���������ށD
 * 
 * @param[in] str	�������ޕ�����i�t�H�[�}�b�g�w��q���g�p�\�j
 */
void logger::Write(const char* str, ...)
{
	va_list args;

	if (fp != NULL){
		long time = timeGetTime();
		fprintf(fp, "time(ms):%ld,", time);
		va_start(args, str);
		vfprintf(fp, str, args);
		va_end(args);
	}
}

/*!
 * @brief ���O�t�@�C���ւ̒ǉ���������
 * �^�C���X�^���v�����ŁC�w�肳�ꂽ�����f�[�^���������ށD
 *
 * @param[in] str	�������ޕ�����i�t�H�[�}�b�g�w��q���g�p�\�j
 */
void logger::WriteWithoutTime(const char* str, ...)
{
	va_list args;

	if (fp != NULL){
		va_start(args, str);
		vfprintf(fp, str, args);
		va_end(args);
	}
}

/*!
 * @brief �I������
 */
void logger::Close()
{
	fclose(fp);
}