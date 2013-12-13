#pragma once

class logger
{
public:
	logger(void);											// �R���X�g���N�^
	~logger(void);											// �f�X�g���N�^
	static void Init(CString filename);						// ������
	static void Write(const char* str, ...);				// ���O�t�@�C���ւ̒ǉ��������݁i�^�C���X�^���v�L�j
	static void WriteWithoutTime(const char* str, ...);		// ���O�t�@�C���ւ̒ǉ��������݁i�^�C���X�^���v���j
	static void Close();									// �I������
protected:
	static FILE* fp;										//! �t�@�C���|�C���^
};

#define LOG( ... ) { logger::Write( __VA_ARGS__ ); }							// ���O�������݃}�N���i�^�C���X�^���v�L�j
#define LOG_WITHOUT_TIME( ... ) { logger::WriteWithoutTime( __VA_ARGS__ ); }	// ���O�������݃}�N���i�^�C���X�^���v���j
