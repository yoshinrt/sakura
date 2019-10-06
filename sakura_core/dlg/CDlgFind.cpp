﻿/*!	@file
	@brief 検索ダイアログボックス

	@author Norio Nakatani
	@date	1998/12/12 再作成
	@date 2001/06/23 N.Nakatani 単語単位で検索する機能を実装
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, JEPRO, hor, Stonee
	Copyright (C) 2002, MIK, hor, YAZAKI, genta
	Copyright (C) 2005, zenryaku
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgFind.h"
#include "view/CEditView.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

//検索 CDlgFind.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//11800
	IDC_BUTTON_SEARCHNEXT,			HIDC_FIND_BUTTON_SEARCHNEXT,		//次を検索
	IDC_BUTTON_SEARCHPREV,			HIDC_FIND_BUTTON_SEARCHPREV,		//前を検索
	IDCANCEL,						HIDCANCEL_FIND,						//キャンセル
	IDC_BUTTON_HELP,				HIDC_FIND_BUTTON_HELP,				//ヘルプ
	IDC_CHK_WORD,					HIDC_FIND_CHK_WORD,					//単語単位
	IDC_CHK_LOHICASE,				HIDC_FIND_CHK_LOHICASE,				//大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_FIND_CHK_REGULAREXP,			//正規表現
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_FIND_CHECK_NOTIFYNOTFOUND,		//見つからないときに通知
	IDC_CHECK_bAutoCloseDlgFind,	HIDC_FIND_CHECK_bAutoCloseDlgFind,	//自動的に閉じる
	IDC_COMBO_TEXT,					HIDC_FIND_COMBO_TEXT,				//検索文字列
	IDC_STATIC_JRE32VER,			HIDC_FIND_STATIC_JRE32VER,			//正規表現バージョン
	IDC_BUTTON_SETMARK,				HIDC_FIND_BUTTON_SETMARK,			//2002.01.16 hor 検索該当行をマーク
	IDC_CHECK_SEARCHALL,			HIDC_FIND_CHECK_SEARCHALL,			//2002.01.26 hor 先頭（末尾）から再検索
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgFind::CDlgFind()
{
	m_sSearchOption.Reset();
	m_nFixedOption = 0;
	return;
}

/*!
	コンボボックスのドロップダウンメッセージを捕捉する

	@date 2013.03.24 novice 新規作成
*/
BOOL CDlgFind::OnCbnDropDown( HWND hwndCtl, int wID )
{
	switch( wID ){
	case IDC_COMBO_TEXT:
		if ( ::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = m_pShareData->m_sSearchKeywords.m_aSearchKeys.size();
			for (int i = 0; i < nSize; ++i) {
				Combo_AddString( hwndCtl, m_pShareData->m_sSearchKeywords.m_aSearchKeys[i] );
			}
		}
		break;
	}
	return CDialog::OnCbnDropDown( hwndCtl, wID );
}

/* モードレスダイアログの表示 */
HWND CDlgFind::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_sSearchOption = m_pShareData->m_Common.m_sSearch.m_sSearchOption;		// 検索オプション
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND;	// 検索／置換  見つからないときメッセージを表示
	m_ptEscCaretPos_PHY = ((CEditView*)lParam)->GetCaret().GetCaretLogicPos();	// 検索開始時のカーソル位置退避
	((CEditView*)lParam)->m_bSearch = TRUE;							// 検索開始位置の登録有無		02/07/28 ai
	return CDialog::DoModeless( hInstance, hwndParent, IDD_FIND, lParam,
		( m_nFixedOption & SCH_BUTTON_MASK ) == SCH_NODLG ? SW_HIDE : SW_SHOW
	);
}

/* モードレス時：検索対象となるビューの変更 */
void CDlgFind::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}

BOOL CDlgFind::OnInitDialog( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	BOOL bRet = CDialog::OnInitDialog(hwnd, wParam, lParam);
	m_comboDel = SComboBoxItemDeleter();
	m_comboDel.pRecent = &m_cRecentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &m_comboDel);

	// フォント設定	2012/11/27 Uchi
	HFONT hFontOld = (HFONT)::SendMessageAny( GetItemHwnd( IDC_COMBO_TEXT ), WM_GETFONT, 0, 0 );
	HFONT hFont = SetMainFont( GetItemHwnd( IDC_COMBO_TEXT ) );
	m_cFontText.SetFont( hFontOld, hFont, GetItemHwnd( IDC_COMBO_TEXT ) );
	return bRet;
}

BOOL CDlgFind::OnDestroy()
{
	m_cFontText.ReleaseOnDestroy();
	return CDialog::OnDestroy();
}

/* ダイアログデータの設定 */
void CDlgFind::SetData( void )
{
//	MYTRACE( L"CDlgFind::SetData()" );

	/*****************************
	*           初期化           *
	*****************************/
	// Here Jun. 26, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直しによりjre.dll判定を削除

	/* ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する */
	// 2011.12.18 長さ制限撤廃
	// Combo_LimitText( GetItemHwnd( IDC_COMBO_TEXT ), _MAX_PATH - 1 );
	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	Combo_SetExtendedUI( GetItemHwnd( IDC_COMBO_TEXT ), TRUE );

	/*****************************
	*         データ設定         *
	*****************************/
	/* 検索文字列 */
	// 検索文字列リストの設定(関数化)	2010/5/28 Uchi
	SetCombosList();

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE,
		m_nFixedOption & SCH_CLR_CASE ? 0 :
		m_nFixedOption & SCH_SET_CASE ? 1 :
		m_sSearchOption.bLoHiCase
	);

	// 2001/06/23 Norio Nakatani
	/* 単語単位で検索 */
	::CheckDlgButton( GetHwnd(), IDC_CHK_WORD,
		m_nFixedOption & SCH_CLR_WORD ? 0 :
		m_nFixedOption & SCH_SET_WORD ? 1 :
		m_sSearchOption.bWordOnly
	);

	/* 検索／置換  見つからないときメッセージを表示 */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND,
		m_nFixedOption & SCH_CLR_MSG ? 0 :
		m_nFixedOption & SCH_SET_MSG ? 1 :
		m_bNOTIFYNOTFOUND
	);

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if(
		CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, false ) && (
			m_nFixedOption & SCH_CLR_REGEXP ? 0 :
			m_nFixedOption & SCH_SET_REGEXP ? 1 :
			m_sSearchOption.bRegularExp
		)
	){
		/* 英大文字と英小文字を区別する */
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 1 );
//正規表現がONでも、大文字小文字を区別する／しないを選択できるように。
//		::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 1 );
//		::EnableWindow( GetItemHwnd( IDC_CHK_LOHICASE ), FALSE );

		// 2001/06/23 N.Nakatani
		/* 単語単位で探す */
		::EnableWindow( GetItemHwnd( IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );
	}
	// To Here Jun. 29, 2001 genta

	/* 検索ダイアログを自動的に閉じる */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bAutoCloseDlgFind,
		m_nFixedOption & SCH_CLR_CLOSEDLG ? 0 :
		m_nFixedOption & SCH_SET_CLOSEDLG ? 1 :
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind
	);

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_SEARCHALL,
		m_nFixedOption & SCH_CLR_LOOP ? 0 :
		m_nFixedOption & SCH_SET_LOOP ? 1 :
		m_pShareData->m_Common.m_sSearch.m_bSearchAll
	);
	
	// デフォルトボタン設定
	if(( m_nFixedOption & SCH_BUTTON_MASK ) == SCH_PREV ) ::PostMessage( GetHwnd(), DM_SETDEFID, IDC_BUTTON_SEARCHPREV, 0 );
	if(( m_nFixedOption & SCH_BUTTON_MASK ) == SCH_NEXT ) ::PostMessage( GetHwnd(), DM_SETDEFID, IDC_BUTTON_SEARCHNEXT, 0 );
	if(( m_nFixedOption & SCH_BUTTON_MASK ) == SCH_MARK ) ::PostMessage( GetHwnd(), DM_SETDEFID, IDC_BUTTON_SETMARK, 0 );
	
	// 検索文字列のみ設定，ダイアログ即閉じる
	if(( m_nFixedOption & SCH_BUTTON_MASK ) == SCH_NODLG ) ::PostMessage(
		GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), BM_CLICK, 0, 0
	);
	
	return;
}

// 検索文字列リストの設定
//	2010/5/28 Uchi
void CDlgFind::SetCombosList( void )
{
	HWND	hwndCombo;

	/* 検索文字列 */
	hwndCombo = GetItemHwnd( IDC_COMBO_TEXT );
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString( hwndCombo, 0);
	}
	int nBufferSize = ::GetWindowTextLength( GetItemHwnd(IDC_COMBO_TEXT) ) + 1;
	auto vText = std::make_unique<WCHAR[]>(nBufferSize);
	Combo_GetText( hwndCombo, &vText[0], nBufferSize );
	if (m_strText.compare( &vText[0] ) != 0) {
		::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_strText.c_str() );
	}
}

/* ダイアログデータの取得 */
int CDlgFind::GetData( void )
{
//	MYTRACE( L"CDlgFind::GetData()" );

	/* 英大文字と英小文字を区別する */
	m_sSearchOption.bLoHiCase = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_LOHICASE ));

	// 2001/06/23 Norio Nakatani
	/* 単語単位で検索 */
	m_sSearchOption.bWordOnly = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_WORD ));

	/* 一致する単語のみ検索する */
	/* 正規表現 */
	m_sSearchOption.bRegularExp = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ));
	if( ::GetKeyState( VK_SHIFT ) & 0x8000 ) m_sSearchOption.bRegularExp = 1;	// shift キーで regexp

	/* 検索／置換  見つからないときメッセージを表示 */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	// 検索／置換  見つからないときメッセージを表示

	/* 検索文字列 */
	int nBufferSize = ::GetWindowTextLength( GetItemHwnd(IDC_COMBO_TEXT) ) + 1;
	auto vText = std::make_unique<WCHAR[]>(nBufferSize);
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_TEXT, &vText[0], nBufferSize);
	m_strText = &vText[0];

	/* 検索ダイアログを自動的に閉じる */
	m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bAutoCloseDlgFind );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	m_pShareData->m_Common.m_sSearch.m_bSearchAll = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SEARCHALL );

	if( 0 < m_strText.length() ){
		/* 正規表現？ */
		// From Here Jun. 26, 2001 genta
		//	正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = m_sSearchOption.bLoHiCase ? 0 : CBregexp::optIgnoreCase;
		if( m_sSearchOption.bRegularExp && !CheckRegexpSyntax( m_strText.c_str(), GetHwnd(), true, nFlag ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え

		/* 検索文字列 */
		//@@@ 2002.2.2 YAZAKI CShareDataに移動
		if( m_strText.size() < _MAX_PATH ){
			CSearchKeywordManager().AddToSearchKeyArr( m_strText.c_str() );
			m_pShareData->m_Common.m_sSearch.m_sSearchOption = m_sSearchOption;		// 検索オプション
		}
		CEditView*	pcEditView = (CEditView*)m_lParam;
		if( pcEditView->m_strCurSearchKey == m_strText && pcEditView->m_sCurSearchOption == m_sSearchOption ){
		}else{
			pcEditView->m_strCurSearchKey = m_strText;
			pcEditView->m_sCurSearchOption = m_sSearchOption;
			pcEditView->m_bCurSearchUpdate = true;
		}
		pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
		if( !m_bModal ){
			/* ダイアログデータの設定 */
			//SetData();
			SetCombosList();		//	コンボのみの初期化	2010/5/28 Uchi
		}
		return 1;
	}else{
		return 0;
	}
}

BOOL CDlgFind::OnBnClicked( int wID )
{
	int			nRet;
	CEditView*	pcEditView = (CEditView*)m_lParam;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「検索」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG) );	//Apr. 5, 2001 JEPRO 修正漏れを追加	// 2006.10.10 ryoji MyWinHelpに変更に変更
		break;
	case IDC_CHK_REGULAREXP:	/* 正規表現 */
//		MYTRACE( L"IDC_CHK_REGULAREXP ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) ){

			// From Here Jun. 26, 2001 genta
			//	正規表現ライブラリの差し替えに伴う処理の見直し
			if( !CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* 英大文字と英小文字を区別する */
				//	Jan. 31, 2002 genta
				//	大文字・小文字の区別は正規表現の設定に関わらず保存する
				//::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 1 );
				//::EnableWindow( GetItemHwnd( IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 Norio Nakatani
				/* 単語単位で検索 */
				::EnableWindow( GetItemHwnd( IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* 英大文字と英小文字を区別する */
			//::EnableWindow( GetItemHwnd( IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	大文字・小文字の区別は正規表現の設定に関わらず保存する
			//::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 Norio Nakatani
			/* 単語単位で検索 */
			::EnableWindow( GetItemHwnd( IDC_CHK_WORD ), TRUE );
		}
		break;
	case IDC_BUTTON_SEARCHPREV:	/* 上検索 */	//Feb. 13, 2001 JEPRO ボタン名を[IDC_BUTTON1]→[IDC_BUTTON_SERACHPREV]に変更
		/* ダイアログデータの取得 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* モーダルダイアログか */
				CloseDialog( 1 );
			}else{
				if( m_nFixedOption & SCH_SETONLY ){
					// 表示だけ更新
					pcEditView->GetCommander().m_pCommanderView->ChangeCurRegexp();
				}else{
					/* 前を検索 */
					pcEditView->GetCommander().HandleCommand( F_SEARCH_PREV, true, (LPARAM)GetHwnd(), 0, 0, 0 );

					/* 再描画 2005.04.06 zenryaku 0文字幅マッチでキャレットを表示するため */
					pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要

					// 02/06/26 ai Start
					// 検索開始位置を登録
					if( FALSE != pcEditView->m_bSearch ){
						// 検索開始時のカーソル位置登録条件変更 02/07/28 ai start
						pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
						pcEditView->m_bSearch = FALSE;
						// 02/07/28 ai end
					}//  02/06/26 ai End
				}

				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
		}
		else if (nRet == 0){
			OkMessage( GetHwnd(), LS(STR_DLGFIND1) );	// 検索条件を指定してください。
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:		/* 下検索 */	//Feb. 13, 2001 JEPRO ボタン名を[IDOK]→[IDC_BUTTON_SERACHNEXT]に変更
		/* ダイアログデータの取得 */
		nRet = GetData();
		if( 0 < nRet ){
			if( m_bModal ){		/* モーダルダイアログか */
				CloseDialog( 2 );
			}
			else{
				if( m_nFixedOption & SCH_SETONLY ){
					// 表示だけ更新
					pcEditView->GetCommander().m_pCommanderView->ChangeCurRegexp();
				}else{
					/* 次を検索 */
					pcEditView->GetCommander().HandleCommand( F_SEARCH_NEXT, true, (LPARAM)GetHwnd(), 0, 0, 0 );

					/* 再描画 2005.04.06 zenryaku 0文字幅マッチでキャレットを表示するため */
					pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要

					// 検索開始位置を登録
					if( FALSE != pcEditView->m_bSearch ){
						// 検索開始時のカーソル位置登録条件変更 02/07/28 ai start
						pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
						pcEditView->m_bSearch = FALSE;
					}
				}

				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
			}
		}
		else if (nRet == 0){
			OkMessage( GetHwnd(), LS(STR_DLGFIND1) );	// 検索条件を指定してください。
		}
		return TRUE;
	case IDC_BUTTON_SETMARK:	//2002.01.16 hor 該当行マーク
		if( 0 < GetData() ){
			if( m_bModal ){		/* モーダルダイアログか */
				CloseDialog( 2 );
			}else{
				pcEditView->GetCommander().HandleCommand( F_BOOKMARK_PATTERN, false, 0, 0, 0, 0 );
				/* 検索ダイアログを自動的に閉じる */
				if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind ){
					CloseDialog( 0 );
				}
				else{
					::SendMessage(GetHwnd(),WM_NEXTDLGCTL,(WPARAM)GetItemHwnd(IDC_COMBO_TEXT ),TRUE);
				}
			}
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgFind::OnActivate( WPARAM wParam, LPARAM lParam )
{
	// 0文字幅マッチ描画のON/OFF	// 2009.11.29 ryoji
	CEditView*	pcEditView = (CEditView*)m_lParam;
	CLayoutRange cRangeSel = pcEditView->GetSelectionInfo().m_sSelect;
	if( cRangeSel.IsValid() && cRangeSel.IsLineOne() && cRangeSel.IsOne() )
		pcEditView->InvalidateRect(NULL);	// アクティブ化／非アクティブ化が完了してから再描画

	return CDialog::OnActivate(wParam, lParam);
}

//@@@ 2002.01.18 add start
LPVOID CDlgFind::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

