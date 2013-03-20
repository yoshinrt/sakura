/*!	@file
	@brief ����֘A

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, �����

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPRINT_H_
#define _CPRINT_H_

#include <WinSpool.h>
#include <CommDlg.h> // PRINTDLG

struct	MYDEVMODE {
	BOOL	m_bPrinterNotFound;	/* �v�����^���Ȃ������t���O */
	TCHAR	m_szPrinterDriverName[_MAX_PATH + 1];	// �v�����^�h���C�o��
	TCHAR	m_szPrinterDeviceName[_MAX_PATH + 1];	// �v�����^�f�o�C�X��
	TCHAR	m_szPrinterOutputName[_MAX_PATH + 1];	// �v�����^�|�[�g��
	DWORD	dmFields;
	short	dmOrientation;
	short	dmPaperSize;
	short	dmPaperLength;
	short	dmPaperWidth;
	short	dmScale;
	short	dmCopies;
	short	dmDefaultSource;
	short	dmPrintQuality;
	short	dmColor;
	short	dmDuplex;
	short	dmYResolution;
	short	dmTTOption;
	short	dmCollate;
	BCHAR	dmFormName[CCHFORMNAME];
	WORD	dmLogPixels;
	DWORD	dmBitsPerPel;
	DWORD	dmPelsWidth;
	DWORD	dmPelsHeight;
	DWORD	dmDisplayFlags;
	DWORD	dmDisplayFrequency;
};

// 2006.08.14 Moca �p�����̓��� PAPER_INFO�V��
//! �p�����
struct PAPER_INFO {
	int				m_nId;			//!< �p��ID
	int				m_nAllWidth;	//!< �� (0.1mm�P��)
	int				m_nAllHeight;	//!< ���� (0.1mm�P��)
	const TCHAR*	m_pszName;		//!< �p������
};

struct PRINTSETTING;




//! ����ݒ�
#define POS_LEFT	0
#define POS_CENTER	1
#define POS_RIGHT	2
#define HEADER_MAX	100
#define FOOTER_MAX	HEADER_MAX
struct PRINTSETTING {
	TCHAR			m_szPrintSettingName[32 + 1];		/*!< ����ݒ�̖��O */
	TCHAR			m_szPrintFontFaceHan[LF_FACESIZE];	/*!< ����t�H���g */
	TCHAR			m_szPrintFontFaceZen[LF_FACESIZE];	/*!< ����t�H���g */
	int				m_nPrintFontWidth;					/*!< ����t�H���g��(1/10mm�P�ʒP��) */
	int				m_nPrintFontHeight;					/*!< ����t�H���g����(1/10mm�P�ʒP��) */
	int				m_nPrintDansuu;						/*!< �i�g�̒i�� */
	int				m_nPrintDanSpace;					/*!< �i�ƒi�̌���(1/10mm�P��) */
	int				m_nPrintLineSpacing;				/*!< ����t�H���g�s�� �����̍����ɑ΂��銄��(%) */
	int				m_nPrintMarginTY;					/*!< ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginBY;					/*!< ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginLX;					/*!< ����p���}�[�W�� ��(mm�P��) */
	int				m_nPrintMarginRX;					/*!< ����p���}�[�W�� �E(mm�P��) */
	int				m_nPrintPaperOrientation;			/*!< �p������ DMORIENT_PORTRAIT (1) �܂��� DMORIENT_LANDSCAPE (2) */
	int				m_nPrintPaperSize;					/*!< �p���T�C�Y */
	bool			m_bPrintWordWrap;					//!< �p�����[�h���b�v����
	bool			m_bPrintKinsokuHead;				//!< �s���֑�����		//@@@ 2002.04.09 MIK
	bool			m_bPrintKinsokuTail;				//!< �s���֑�����		//@@@ 2002.04.09 MIK
	bool			m_bPrintKinsokuRet;					//!< ���s�����̂Ԃ牺��	//@@@ 2002.04.13 MIK
	bool			m_bPrintKinsokuKuto;				//!< ��Ǔ_�̂Ԃ炳��	//@@@ 2002.04.17 MIK
	BOOL			m_bPrintLineNumber;					/*!< �s�ԍ���������� */


	MYDEVMODE		m_mdmDevMode;						/*!< �v�����^�ݒ� DEVMODE�p */
	BOOL			m_bHeaderUse[3];					/* �w�b�_���g���Ă��邩�H	*/
	EDIT_CHAR		m_szHeaderForm[3][HEADER_MAX];		/* 0:���񂹃w�b�_�B1:�����񂹃w�b�_�B2:�E�񂹃w�b�_�B*/
	BOOL			m_bFooterUse[3];					/* �t�b�^���g���Ă��邩�H	*/
	EDIT_CHAR		m_szFooterForm[3][FOOTER_MAX];		/* 0:���񂹃t�b�^�B1:�����񂹃t�b�^�B2:�E�񂹃t�b�^�B*/
};


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief ����֘A�@�\

	�I�u�W�F�N�g�w���łȂ��N���X
*/
class CPrint
{
public:
	static const PAPER_INFO m_paperInfoArr[];	//!< �p�����ꗗ
	static const int m_nPaperInfoArrNum; //!< �p�����ꗗ�̗v�f��


	/*
	||	static�֐��Q
	*/
	static void SettingInitialize( PRINTSETTING&, const TCHAR* settingName );

	static TCHAR* GetPaperName( int , TCHAR* );	/* �p���̖��O���擾 */
	/* �p���̕��A���� */
	static BOOL GetPaperSize(
		int*		pnPaperAllWidth,
		int*		pnPaperAllHeight,
		MYDEVMODE*	pDEVMODE
	);

public:
	/*
	||  Constructors
	*/
	CPrint();
	~CPrint();

	/*
	||  Attributes & Operations
	*/
	BOOL GetDefaultPrinter( MYDEVMODE *pMYDEVMODE );		/* �f�t�H���g�̃v�����^�����擾 */
	BOOL PrintDlg( PRINTDLG *pd, MYDEVMODE *pMYDEVMODE );				/* �v�����^�����擾 */
	/* ���/�v���r���[�ɕK�v�ȏ����擾 */
	BOOL GetPrintMetrics(
		MYDEVMODE*	pMYDEVMODE,
		int*		pnPaperAllWidth,	/* �p���� */
		int*		pnPaperAllHeight,	/* �p������ */
		int*		pnPaperWidth,		/* �p������\�� */
		int*		pnPaperHeight,		/* �p������\���� */
		int*		pnPaperOffsetLeft,	/* �p���]�����[ */
		int*		pnPaperOffsetTop,	/* �p���]����[ */
		TCHAR*		pszErrMsg			/* �G���[���b�Z�[�W�i�[�ꏊ */
	);


	/* ��� �W���u�J�n */
	BOOL PrintOpen(
		TCHAR*		pszJobName,
		MYDEVMODE*	pMYDEVMODE,
		HDC*		phdc,
		TCHAR*		pszErrMsg		/* �G���[���b�Z�[�W�i�[�ꏊ */
	);
	void PrintStartPage( HDC );	/* ��� �y�[�W�J�n */
	void PrintEndPage( HDC );	/* ��� �y�[�W�I�� */
	void PrintClose( HDC );		/* ��� �W���u�I�� */ // 2003.05.02 ����� �s�v��hPrinter�폜

protected:
	/*
	||  �����w���p�֐�
	*/
	// DC�쐬����(�������܂Ƃ߂�) 2003.05.02 �����
	HDC CreateDC( MYDEVMODE *pMYDEVMODE, TCHAR *pszErrMsg);
	
	static const PAPER_INFO* FindPaperInfo( int id );
private:
	/*
	||  �����o�ϐ�
	*/
	HGLOBAL	m_hDevMode;							//!< ���݃v�����^��DEVMODE�ւ̃������n���h��
	HGLOBAL	m_hDevNames;						//!< ���݃v�����^��DEVNAMES�ւ̃������n���h��
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPRINT_H_ */


