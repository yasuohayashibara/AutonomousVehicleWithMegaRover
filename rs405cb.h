#define SERVO_ID	1							// �T�[�{��ID
#define	COM_PORT	"COM1"						// �T�[�{��COM�ԍ�
#define	MAX_TORQUE	0x64						// �ő�g���N

HANDLE CommOpen( char *pport );					// �ʐM�|�[�g�̃I�[�v��
int CommClose( HANDLE hComm );					// �ʐM�|�[�g�����
int RSMove( HANDLE hComm, short sPos, unsigned short sTime );
												// �T�[�{�̏o�͊p���w��
short RSStartGetAngle( HANDLE hComm );			// �T�[�{�̌��݊p�x�̎擾���J�n����
short RSGetAngle( HANDLE hComm );				// �T�[�{�̌��݊p�x���擾����
int RSTorqueOnOff( HANDLE hComm, short sMode );	// �T�[�{�̃g���N��ON/OFF����
int RSMaxTorque( HANDLE hComm, int maxTorque );	// �T�[�{�̃g���N��ݒ肷��
