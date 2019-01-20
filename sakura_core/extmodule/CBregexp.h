﻿/*!	@file
	@brief BREGEXP Library Handler

	pcre2 を利用するためのインターフェース

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef _DLL_BREGEXP_H_
#define _DLL_BREGEXP_H_

#include "CDllHandler.h"

class CBregexp {
public:
	CBregexp();
	~CBregexp();

	// 2006.01.22 かろと オプション追加・名称変更
	enum Option {
		optNothing = 0,					//!< オプションなし
		optCaseSensitive = 1,			//!< 大文字小文字区別オプション(/iをつけない)
		optGlobal = 2,					//!< 全域オプション(/g)
		optExtend = 4,					//!< 拡張正規表現(/x)
		optASCII = 8,					//!< ASCII(/a)
		optUnicode = 0x10,				//!< Unicode(/u)
		optDefault = 0x20,				//!< Default(/d)
		optLocale = 0x40,				//!< Locale(/l)
		optR = 0x80,					//!< CRLF(/R)
	};

	//! DLLのバージョン情報を取得
	const TCHAR* GetVersionT(){ return _T(""); }

	//	CJreエミュレーション関数
	//!	検索パターンのコンパイル
	// 2002/01/19 novice 正規表現による文字列置換
	// 2002.01.26 hor    置換後文字列を別引数に
	// 2002.02.01 hor    大文字小文字を無視するオプション追加
	//>> 2002/03/27 Azumaiya 正規表現置換にコンパイル関数を使う形式を追加
	bool Compile(const wchar_t *szPattern, int nOption = 0) {
		return Compile(szPattern, NULL, nOption);
	}
	bool Compile(const wchar_t *szPattern0, const wchar_t *szPattern1, int nOption = 0, bool bKakomi = false);	//!< Replace用
	bool Match(const wchar_t *szTarget, int nLen, int nStart = 0);						//!< 検索を実行する
	int Replace(const wchar_t *szTarget, int nLen, int nStart = 0);					//!< 置換を実行する	// 2007.01.16 ryoji 戻り値を置換個数に変更

	//-----------------------------------------
	// 2005.03.19 かろと クラス内部を隠蔽
	/*! @{
		@name 結果を得るメソッドを追加し、BREGEXPを外部から隠す
	*/
	/*!
	    検索に一致した文字列の先頭位置を返す(文字列先頭なら0)
		@retval 検索に一致した文字列の先頭位置
	*/
	CLogicInt GetIndex( void ){
		return CLogicInt( pcre2_get_ovector_pointer( m_MatchData )[ 0 ]);
	}
	/*!
	    検索に一致した文字列の次の位置を返す
		@retval 検索に一致した文字列の次の位置
	*/
	CLogicInt GetLastIndex(void)
	{
		return CLogicInt( pcre2_get_ovector_pointer( m_MatchData )[ 1 ] );
	}
	/*!
		検索に一致した文字列の長さを返す
		@retval 検索に一致した文字列の長さ
	*/
	CLogicInt GetMatchLen(void)
	{
		int *p = ( int *)pcre2_get_ovector_pointer( m_MatchData );
		return CLogicInt( p[ 1 ] - p[ 0 ]);
	}
	/*!
		置換された文字列の長さを返す
		@retval 置換された文字列の長さ
	*/
	CLogicInt GetStringLen(void) {
		// 置換後文字列が０幅なら outp、outendpもNULLになる
		// NULLポインタの引き算は問題なく０になる。
		// outendpは '\0'なので、文字列長は +1不要
		return CLogicInt(0);	// ★未実装
#ifdef HOGE
		// Jun. 03, 2005 Karoto
		//	置換後文字列が0幅の場合にoutpがNULLでもoutendpがNULLでない場合があるので，
		//	outpのNULLチェックが必要

		if (m_pRegExp->outp == NULL) {
			return CLogicInt(0);
		} else {
			return CLogicInt(m_pRegExp->outendp - m_pRegExp->outp);
		}
#endif
	}
	/*!
		置換された文字列を返す
		@retval 置換された文字列へのポインタ
	*/
	const wchar_t *GetString(void)
	{
		return L"";	// ★未実装
		//return m_pRegExp->outp;
	}
	/*! @} */
	//-----------------------------------------

	/*! BREGEXPメッセージを取得する
		@retval メッセージへのポインタ
	*/
	const TCHAR* GetLastMessage() const { return to_tchar( m_szMsg ); }

protected:

	//!	コンパイルバッファを解放する
	/*!
		m_MatchData, m_Re を解放する．解放後はNULLにセットする．
		元々NULLなら何もしない
	*/
	void ReleaseCompileBuffer(void){
		if( m_MatchData ) pcre2_match_data_free( m_MatchData );
		m_MatchData = nullptr;
		
		if( m_Re ) pcre2_code_free( m_Re );
		m_Re = nullptr;
	}

private:
	//	内部関数

	//! 検索パターン作成
	wchar_t* MakePatternSub( const wchar_t* szPattern, const wchar_t* szPattern2, const wchar_t* szAdd2, int nOption );
	wchar_t* MakePattern( const wchar_t* szPattern, const wchar_t* szPattern2, int nOption );
	wchar_t* MakePatternAlternate( const wchar_t* const szSearch, const wchar_t* const szReplace, int nOption );

	//	メンバ変数
	const wchar_t*		m_szTarget;			//!< 対象文字列へのポインタ
	
	static const int	MSGBUF_SIZE	= 80;
	wchar_t				m_szMsg[ MSGBUF_SIZE ];	//!< BREGEXP_Wからのメッセージを保持する

	// 静的メンバ変数
	static const wchar_t	m_tmpBuf[2];	//!< ダミー文字列
	
	// pcre2
	pcre2_match_data	*m_MatchData;
	pcre2_code			*m_Re;
	
	// bregonig.dll I/F
public:
	// DLL の名残
	bool IsAvailable( void ){ return true; }
	
	//! DLLロードと初期処理
	EDllResult InitDll(
		LPCTSTR pszSpecifiedDllName = NULL	//!< [in] クラスが定義しているDLL名以外のDLLを読み込みたいときに、そのDLL名を指定。
	){
		return DLL_SUCCESS;
	}
};

//	Jun. 26, 2001 genta
//!	正規表現ライブラリのバージョン取得
static inline bool CheckRegexpVersion( HWND hWnd, int nCmpId, bool bShowMsg = false ){ return true; };
bool CheckRegexpSyntax( const wchar_t* szPattern, HWND hWnd, bool bShowMessage, int nOption = -1, bool bKakomi = false );// 2002/2/1 hor追加
static inline bool InitRegexp( HWND hWnd, CBregexp& rRegexp, bool bShowMessage ){
	return true;
}

#endif
