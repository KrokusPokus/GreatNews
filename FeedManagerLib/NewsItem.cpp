#include "StdAfx.h"
#include "RxUtil.h"
#include "NewsItem.h"
#include "FeedManager.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "FeedManagerLibHelper.h"
#include "NewsItemCache.h"
#include "NewsWatchCache.h"
#include "GMTimeLib.h"
#include "NewsFeedCache.h"
#include "GNUtil.h"
#include "zutil.h"

long CNewsItem::m_selectedFeatures;

void CNewsItem::SetItemFeatures(long selectedFeatures, bool bHighlightNewsWatch)
{
	m_selectedFeatures = selectedFeatures;
	if(bHighlightNewsWatch)
		m_selectedFeatures |= HighlightNewsWatch;
}

// STL utils
class CContructIDList
{
public:
	void operator()(ULONG_PTR id)
	{
		m_IDList.AppendFormat(_T("%d,"), id);
	}

	CString GetIDList()
	{
		if(m_IDList.GetLength())
			return m_IDList.Left(m_IDList.GetLength()-1); // remove the last comma
		else
			return m_IDList;
	}

private:
	CString m_IDList;
};


CNewsItem::CNewsItem(void):
	m_id(0),
	m_feedID(0),
	m_notRead(true), 
	m_marked(0),
	m_pCompressStream(NULL),
	m_compressSize(0),
	m_bWatchChecked(false),
	m_bkColor((COLORREF)DefaultColor),
	m_textColor((COLORREF)DefaultColor),
	m_bTrackComments(false),
	m_contentCRC(0)
{
}

CNewsItem::~CNewsItem(void)
{
	CleanCompress();
}

bool CNewsItem::WatchStatusChecked()
{
	return m_bWatchChecked;
}

void CNewsItem::CleanCompress()
{
	m_compressSize = 0;
	if(m_pCompressStream)
	{
		delete[] m_pCompressStream;
		m_pCompressStream = NULL;
	}
}

void CNewsItem::Init(ULONG_PTR id)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from news_item where news_id=%d"), CFeedManagerLibHelper::m_newsItemFields, id);
		CppSQLite3Query q = db.execQuery(sql);
		if(q.eof())
		{
			q.finalize();
			throw CExceptionBase(ERR_FM_DBERROR, _T("Specified news item cannot be found."));
		}
		else
		{
			CFeedManagerLibHelper::PopulateNewsItem(this, q);
			q.finalize();
		}
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}

CNewsItem* CNewsItem::Clone() const
{
	CNewsItem* pItem = new CNewsItem();

	pItem->m_id = m_id;
	pItem->m_feedID = m_feedID;
	pItem->m_url = m_url;
	pItem->m_title = m_title;
	pItem->m_description = m_description;
	pItem->m_author = m_author;
	pItem->m_date = m_date;
	pItem->m_notRead = m_notRead;
	pItem->m_marked = m_marked;
	pItem->m_guid = m_guid;
	pItem->m_podCastingURL = m_podCastingURL;
	pItem->m_commentFeedURL = m_commentFeedURL;
	pItem->m_bTrackComments = m_bTrackComments;

	return pItem;
}

//CString CNewsItem::Base64Encode(const CString& sSource)
//{
//	int nDestLen = Base64EncodeGetRequiredLength(sSource.GetLength());
//	CString str64;
//	::Base64Encode((const BYTE*)(LPCTSTR)sSource, sSource.GetLength(), 
//				str64.GetBuffer(nDestLen), &nDestLen);
//	str64.ReleaseBuffer(nDestLen);
//	return str64;
//}

//CString CNewsItem::Base64Decode(const CString& str64)
//{
//	int nDecLen = Base64DecodeGetRequiredLength(str64.GetLength());
//	CString strOrig;
//	::Base64Decode(str64, str64.GetLength(), (BYTE*)strOrig.GetBuffer(nDecLen),&nDecLen);
//	strOrig.ReleaseBuffer(nDecLen);
//
//	return strOrig;
//}


void CNewsItem::Save()
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		ATLASSERT(m_feedID > 0);

		CString sql;
		if(m_id <= 0)
		{
			// insert
			CppSQLite3Statement stmt = db.compileStatement(
				_T("insert into news_item (")
				_T("feed_id,")		// 1
				_T("url,")			// 2
				_T("title,")		// 3
				_T("description,")	// 4
				_T("retrieved,")	// 5
				_T("item_guid,")	// 6
				_T("mark,")			// 7
				_T("readtime,")		// 8
				_T("author,")		// 9
				_T("pod_url,")		// 10
				_T("commentRss")	// 11
				_T(") values (?,?,?,?,?,?,?,?,?,?,?);"));
			stmt.bind(1,m_feedID);
			stmt.bind(2,(LPCTSTR)m_url);
			stmt.bind(3,(LPCTSTR)m_title);
			//stmt.bind(4,(LPCTSTR)m_description);
			if (m_description.GetLength() == 0)
			{
				stmt.bindNull(4);
			}
			else
			{
				Compress();
				stmt.bind(4, GetCompressStream(), GetCompressLen());
				CleanCompress();
			}
			stmt.bind(5,(int)m_date.GetTime());
			stmt.bind(6,(LPCTSTR)m_guid);
			if(m_marked<=0)
				stmt.bindNull(7);
			else
				stmt.bind(7, m_marked);
			if(m_notRead)
				stmt.bindNull(8);
			else
				stmt.bind(8, m_notRead);
			if(m_author.GetLength()==0)
				stmt.bindNull(9);
			else
				stmt.bind(9, (LPCTSTR)m_author);
			if(m_podCastingURL.GetLength()==0)
				stmt.bindNull(10);
			else
				stmt.bind(10, (LPCTSTR)m_podCastingURL);
			if(m_commentFeedURL.GetLength()==0)
				stmt.bindNull(11);
			else
				stmt.bind(11, (LPCTSTR)m_commentFeedURL);
			db.execDML(_T("begin immediate transaction;"));
			stmt.execDML();
			m_id = (LONG_PTR)db.lastRowId();
			db.execDML(_T("commit transaction;"));
		}
		else
		{
			// update
			CppSQLite3Statement stmt = db.compileStatement(
				_T("update news_item set ")
				_T("feed_id = ?,")			// 1
				_T("url = ?,")				// 2
				_T("title = ?,")			// 3
				_T("description = ?,")		// 4
				_T("retrieved = ?,")		// 5
				_T("item_guid = ?,")		// 6
				_T("mark = ?,")				// 7
				_T("readtime = ?,")			// 8
				_T("author = ?,")			// 9
				_T("pod_url = ?,")			// 10
				_T("commentRss = ? ")		// 11
				_T("where news_id = ?;"));	// 12
			stmt.bind(1, m_feedID);
			stmt.bind(2,(LPCTSTR)m_url);
			stmt.bind(3,(LPCTSTR)m_title);
			//stmt.bind(4,(LPCTSTR)m_description);
			if (m_description.GetLength() == 0)
			{
				stmt.bindNull(4);
			}
			else
			{
				Compress();
				stmt.bind(4, GetCompressStream(), GetCompressLen());
				CleanCompress();
			}
			stmt.bind(5,(int)m_date.GetTime());
			stmt.bind(6,(LPCTSTR)m_guid);
			if(m_marked <= 0)
				stmt.bindNull(7);
			else
				stmt.bind(7, m_marked);
			if(m_notRead)
				stmt.bindNull(8);
			else
				stmt.bind(8, m_notRead);
			if(m_author.GetLength()==0)
				stmt.bindNull(9);
			else
				stmt.bind(9, (LPCTSTR)m_author);
			if(m_podCastingURL.GetLength()==0)
				stmt.bindNull(10);
			else
				stmt.bind(10, (LPCTSTR)m_podCastingURL);
			if(m_commentFeedURL.GetLength()==0)
				stmt.bindNull(11);
			else
				stmt.bind(11, (LPCTSTR)m_commentFeedURL);
			stmt.bind(12,m_id);
			stmt.execDML();
		}
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

}

void CNewsItem::SetRead()
{
	m_notRead = 0;
	CString sql;
	sql.Format(_T("update news_item set readtime = 1 where news_id = %d and readtime is null;"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

void CNewsItem::SetUnread()
{
	m_notRead = 1;
	CString sql;
	sql.Format(_T("update news_item set readtime = null where news_id = %d and readtime is not null;"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}


bool CNewsItem::IsUnread()
{
	return (m_notRead != 0);
}


void CNewsItem::InitGenerator(int nPageSize)
{
}

CString CNewsItem::GeneratePageHTML(int)
{
	return GeneratePageContentHTML(0);
}

CString CNewsItem::GeneratePageContentHTML(int)
{
	m_unreadItemIDs.clear();
	m_channelUnreadCounts.clear();

	if(IsUnread())
	{
		m_unreadItemIDs.push_back(m_id);
		m_channelUnreadCounts.insert(std::map<ULONG_PTR, int>::value_type(m_feedID, 1));
	}

	return internalGenerateHTML(true);
}

CString CNewsItem::GetLink() const
{
	return m_url.GetLength() ? m_url :
		m_guid.Find(_T("http://")) != -1 ? m_guid : _T("");
}

CString CNewsItem::GetTitle() const
{
	if(m_title.GetLength())
		return m_title;
	
	CString title;
	CString desc = m_description;
	if(desc.GetLength() == 0)
		return _T("");

	// remove all html tags first
	CRxUtil<CAtlRECharTraits> myRxUtil;
	myRxUtil.s(desc, _T("<.+?>"),_T(" "),true);

	int len = min(100, desc.GetLength());

	title = desc.Left(len)+_T("...");
	title.TrimLeft();
	return title;
}

CString CNewsItem::GetLabelThisUrl() const
{
	CString url;
	url.Format(_T("#GreatNewsTag_LabelItem_%d"), m_id);
	return url;
}

LPCTSTR CNewsItem::GetLabelThisUrlClass() const
{
	return (m_marked>0 ? _T("withLabel") : _T("noLabel"));
}

CString CNewsItem::GetTrackCommentUrl() const
{
	CString url;
	url.Format(_T("#GreatNewsTag_TrackComments_%d"), m_id);
	return url;
}

LPCTSTR CNewsItem::GetTrackCommentText() const
{
	return (m_bTrackComments ? _T("Stop Tracking Comments") : _T("Track Comments"));
}

CString CNewsItem::GetDivID() const
{
	CString divID;
	divID.Format(_T("item%d"), m_id);
	return divID;
}

CString CNewsItem::internalGenerateHTML(bool bSingleItem)
{
	CString podUrl;
	if(m_podCastingURL.GetLength())
	{
		podUrl.Format(_T("<div class=\"gn_podurl\"><a class=\"gn_podurl\" href=\"%s\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</a></div>\n"), (LPCTSTR)m_podCastingURL);
	}

	CString desc = m_description;
	//if(desc.GetLength())
	//{
	//	CRxUtil<CAtlRECharTraits> myRxUtil;
	//	// remove comments
	//	myRxUtil.s(desc, _T("<\\!--(.*?|\\n*?)-->"),_T(" "),true, false, false);
	//	// remove some tags
	//	myRxUtil.s(desc, _T("<(\\b)*?(embed)|(script)|(object)|(iframe)|(meta)|(frame)|(frameset)|(link)(.*?|\\n*?)>"),_T(" "),true, false, false);
	//	// remove attributes
	//	myRxUtil.s(desc, _T("{<(.*?|\\n*?)(\\b)+?}(style)|(onabort)|(onblur)|(onchange)|(onclick)|(ondblclick)|(onfocus)|(onkeydown)|(onkeypress)|(onkeyup)|(onload)|(onmousedown)|(onmousemove)|(onmouseover)|(onmouseout)|(onmouseup)|(onreset)|(onselect)|(onsubmit)|(onunload){(\\b)*?=(.*?|\\n*?)>}"),_T("$1xxxx$2"),true, true, false);
	//	// remove style tag
	//	myRxUtil.s(desc, _T("<(\\b)*?style(.*?|\\n*?)>(.*?|\\n*?)<(\\b)*?/(\\b)*?style(\\b)*?>"),_T(" "),true, false, false);
	//}

	CString style;
	if(m_selectedFeatures & CNewsItem::HighlightNewsWatch)
	{
		UpdateWatchColor();
		COLORREF txtColor = GetTextColor();
		if(txtColor!=CNewsItem::DefaultColor)
			style.AppendFormat(_T("color:#%02x%02x%02x;"),GetRValue(txtColor),GetGValue(txtColor),GetBValue(txtColor));

		COLORREF bkColor = GetBkColor();
		if(bkColor!=CNewsItem::DefaultColor)
		{
			style.AppendFormat(_T("background-color:#%02x%02x%02x;"),GetRValue(bkColor),GetGValue(bkColor),GetBValue(bkColor));
		}
	}

	// generate html
	CString html;
	CString readFlag;
	readFlag.Format(_T("<span class=\"%s\"><a href=\"#GreatNewsTag_ToggleRead_%d\" title=\"Toggle read/unread\">&nbsp;&nbsp;&nbsp;</a></span>"),
		IsUnread() ? _T("unreaditem") : _T("readitem"),
		m_id);
	html.AppendFormat(_T("<div id=\"%s\" class=\"%s%s\" style=\"%s\">\n")
					_T("<div class=\"gn_storytitle\">%s<a href=\"%s\" class=\"gn_storytitle\">%s</a></div>\n")
					_T("<div class=\"gn_storydetails\">%s%s</div>\n")
					_T("<div class=\"gn_storyfooter\"><span class=\"gn_publishtime\">%s</span>\n"),
		(LPCTSTR)GetDivID(),
		bSingleItem ? _T("gn_story") : _T("gn_stories"),
		IsUnread() ? _T(" gn_unreaditem") : _T(" gn_readitem"),
		(LPCTSTR)style,
		(LPCTSTR)readFlag,
		(LPCTSTR)GetLink(),
		(LPCTSTR)GetTitle(),
		(LPCTSTR)podUrl,
		(LPCTSTR)desc,
		(LPCTSTR)CGMTimeHelper::FormatDisplayDate(m_date));

	// generate various features
	if(m_selectedFeatures & CNewsItem::LabelThis)
	{
		html.AppendFormat(_T(" | <a class=\"%s\" href=\"%s\">Label This</a>"),
							GetLabelThisUrlClass(),
							(LPCTSTR)GetLabelThisUrl());
	}

	if(m_selectedFeatures & CNewsItem::EmailThis)
	{
		html.AppendFormat(_T(" | <a class=\"noLabel\" href=\"http://localhost/#GreatNewsTag_EmailItem_%d\">Email This</a>"),
							m_id);
	}

	if(m_selectedFeatures & CNewsItem::BlogThis)
	{
		html.AppendFormat(_T(" | <a class=\"noLabel\" href=\"http://localhost/#GreatNewsTag_BlogThis_%d\">Blog This</a>"),
							m_id);
	}

	if(m_selectedFeatures & CNewsItem::Delicious)
	{
		html.AppendFormat(_T(" | <a class=\"noLabel\" href=\"http://localhost/#GreatNewsTag_AddDelicious_%d\">+Del.icio.us</a>"),
							m_id);
	}

	if(m_selectedFeatures & CNewsItem::Furl)
	{
		html.AppendFormat(_T(" | <a class=\"noLabel\" href=\"http://localhost/#GreatNewsTag_AddFurl_%d\">+Furl</a>"),
							m_id);
	}

	if((m_selectedFeatures & CNewsItem::Arthur)
		&& m_author.GetLength()>0)
	{
		html.AppendFormat(_T(" | <span class=\"gn_publishtime\">by %s</span>"),
							(LPCTSTR)m_author);
	}


	if((m_selectedFeatures & CNewsItem::TrackComments)
		&& m_commentFeedURL.GetLength()>0)
	{
		html.AppendFormat(_T(" | <a class=\"noLabel\" href=\"%s\">%s</a>"),
							(LPCTSTR)GetTrackCommentUrl(),
							(LPCTSTR)GetTrackCommentText());
	}

	html.Append(_T("</div>\n"));


	//
	// now check if we have comments
	//
	if(m_bTrackComments)
	{
		NewsFeedPtr commentFeed = CNewsFeedCache::GetCommentFeed(m_id);
		if(commentFeed)
		{
			std::vector<ULONG_PTR> vectNewsIDs;
			if(commentFeed->GetNewsItemIDs(vectNewsIDs, m_pNewsFilter))
			{
				CNewsItemCache::FillCache(vectNewsIDs, true);

				for(std::vector<ULONG_PTR>::iterator it = vectNewsIDs.begin(); it != vectNewsIDs.end(); ++it)
				{
					NewsItemPtr& spItem = CNewsItemCache::GetNewsItem(*it);
					html.AppendFormat(_T("<div class=\"comment\"><span class=\"commentTime\">%s"),
						(LPCTSTR)CGMTimeHelper::FormatDisplayDate(spItem->m_date));
					if(!spItem->m_author.IsEmpty())
						html.AppendFormat(_T(" by %s"), (LPCTSTR)spItem->m_author);
					html.Append(_T("</span>")); //close time span

					if(!spItem->m_title.IsEmpty())
					{
						if(!spItem->m_url.IsEmpty())	// track back?
						{
							html.AppendFormat(_T("<span class=\"commentContent\"><a href=\"%s\">%s</a></span>"),
									(LPCTSTR)spItem->m_url,
									(LPCTSTR)spItem->m_title);
						}
						else
						{
							html.AppendFormat(_T("<span class=\"commentContent\">%s</span>"),
									(LPCTSTR)spItem->m_title);
						}
					}
					html.Append(_T("<br/>"));

					html.AppendFormat(_T("<span class=\"commentContent\">%s</span></div>\n"),
						(LPCTSTR)spItem->m_description);
				}
			}
			else
			{
				html.AppendFormat(_T("<div class=\"comment\"><span class=\"commentContent\">No comments yet</span></div>\n"));
			}
		}
	}

	// close it up
	html.Append(_T("</div>\n"));
	return html;
}

CString CNewsItem::GetContentID() const
{
	CString temp;
	temp.Format(_T("I%d"), m_id);
	return temp;
}

void CNewsItem::MarkAllAsRead()
{
	CNewsItemCache::Empty();

	CString sql;
	sql.Format(_T("update news_item set readtime=1 where feed_id > 0 and readtime is null;"));

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	CNewsItemCache::MarkAllRead();
}

void CNewsItem::ClearWatch()
{
	m_bWatchChecked = false;
	m_bkColor = (COLORREF)DefaultColor;
	m_textColor = (COLORREF)DefaultColor;
}

bool CNewsItem::UpdateWatchColor()
{
	if(m_bWatchChecked == true)
		return true;

	if(LoadWatchIDs(m_vectWatches)==0)
	{
		return false;
	}

	return CalculateColors();
}

CString CNewsItem::GetStyle() const
{
	try
	{
		NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(m_feedID);
		return feed->m_channelStyle;
	}
	catch(CppSQLite3Exception&)
	{
	}
	
	return "";
}

CString CNewsItem::GetHomeURL() const
{
	try
	{
		NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(m_feedID);
		return feed->m_website;
	}
	catch(CppSQLite3Exception&)
	{
	}
	
	return "";
}

size_t CNewsItem::GetNewsItemsByID(NewsItemVector& items, const std::vector<ULONG_PTR>& IDs)
{
	items.clear();

	if(IDs.empty())
		return 0;

	items.reserve(IDs.size());

	CContructIDList idList;
	idList = std::for_each(IDs.begin(), IDs.end(), idList);

	CString criteria;
	criteria.Format(_T(" news_id in (%s) "), (LPCTSTR)idList.GetIDList());

	return CFeedManagerLibHelper::RetrieveNewsFromDB(items, criteria, NULL, _T(""));
}

size_t CNewsItem::GetItemTags(std::vector<ULONG_PTR>& tagIDs)
{
	tagIDs.clear();
	if(m_id==0)
		return 0;

	tagIDs.reserve(20);

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);
	try
	{
		CString sql;
		sql.Format(_T("select tag_id from item_tag where news_id = %d;"), m_id);
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			tagIDs.push_back((ULONG_PTR)q.getIntField(0));
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return tagIDs.size();
}


void CNewsItem::AddTag(ULONG_PTR tagID)
{
	CString sql;
	sql.Format(_T("insert into item_tag (tag_id, news_id) values(%d,%d); \
					update news_item set mark=ifnull(mark,0)+1 where news_id=%d;\
					update feed set usage=ifnull(usage,0)+2 where feed_id=%d;"), 
				tagID, m_id, m_id, m_feedID);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	m_marked++;
}

void CNewsItem::RemoveTag(ULONG_PTR tagID)
{
	CString sql;
	// trigger will change the item label count. we need to do this in trigger because user can
	// delete the whole label.
	sql.Format(_T("delete from item_tag where tag_id=%d and news_id=%d;"), 
				tagID, m_id, m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	m_marked--;
}

CString CNewsItem::GetUnescapedTitle() const
{
	CString title = m_title;
	CGNUtil::UnescapeHtml(title);
	return title;
}

void CNewsItem::UnescapeTitle()
{
	CGNUtil::UnescapeHtml(m_title);
}

void CNewsItem::CalcContentCRC()
{
	if(!m_description.IsEmpty())
	{
		// calculate data crc
		uLong crc = crc32(0L, Z_NULL, 0);
		crc = crc32(crc, (LPBYTE)(LPCTSTR)m_description, m_description.GetLength()*sizeof(TCHAR));
		m_contentCRC = crc;
	}
	else
	{
		m_contentCRC = 0;
	}
}

void CNewsItem::Compress()
{
	RemoveDangerousTags();

	CleanCompress();

	ULONG inLen = (m_description.GetLength()+1)*sizeof(WCHAR);
	if(inLen == 0)
		return;

	ULONG bufferSize = ULONG(inLen*1.1)+100; 
	BYTE* out = new BYTE[sizeof(ULONG) + bufferSize];  // the first 4 bytes are for the uncompressed len
	*(ULONG*)out = inLen;
	USES_CONVERSION;
	LPCWSTR pString = T2W((LPTSTR)(LPCTSTR)m_description);
	int ret = compress2(out+sizeof(ULONG), &bufferSize, (const BYTE*)pString, inLen, 9);

	if(ret != Z_OK)
	{
		delete[] out;
		CString msg;
		msg.Format(_T("Failed to compress due to error[%d]."), ret);
		throw CExceptionBase(ERR_FM_DBERROR, msg);
	}

	m_pCompressStream = out;
	m_compressSize = bufferSize+sizeof(ULONG);
}

void CNewsItem::DecompressDescription(const BYTE* pStream, int size)
{
	m_description.Empty();
	if(!pStream || !size)
		return;

#define BUFFER_SIZE 32000

	BYTE fixedBuffer[BUFFER_SIZE];
	BYTE* pUncompressBuffer = fixedBuffer;
	BYTE* pDynBuffer = NULL;
	ULONG uncompressLen = BUFFER_SIZE;

	// check if the default buffer is big enough
	ULONG originalSize = *(ULONG*)pStream;
	if(originalSize>=BUFFER_SIZE)
	{
		AtlTrace(_T("Default buffer is not enough. Need %d\n"),originalSize); 
		pDynBuffer = new BYTE[originalSize];
		pUncompressBuffer = pDynBuffer;
		uncompressLen = originalSize;
	}

	int ret = uncompress(pUncompressBuffer, &uncompressLen,pStream+sizeof(ULONG), size-sizeof(ULONG));

	if(ret != Z_OK)
	{
		if(pDynBuffer)
			delete[] pDynBuffer;
		
		CString msg;
		msg.Format(_T("Failed to decompress due to error[%d]."), ret);
		throw CExceptionBase(ERR_FM_DBERROR, msg);
	}

	ATLASSERT(uncompressLen == originalSize);

#ifdef UNICODE
	m_description = ((LPCWSTR)pUncompressBuffer);
#else
	// have to convert unicode to UTF8
	int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0,(LPCWSTR)pUncompressBuffer, -1, NULL, NULL, NULL, NULL);
	LPSTR lpszA = new char[nUtf8Len];
	WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pUncompressBuffer, -1, lpszA, nUtf8Len, NULL, NULL);
	m_description = lpszA;
	// free the string
	delete[] lpszA;
#endif

	if(pDynBuffer)
		delete[] pDynBuffer;
}

size_t CNewsItem::LoadWatchIDs(std::vector<ULONG_PTR>& watches)
{
	try
	{
		watches.clear();
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql;
		sql.Format(_T("select watch_id from watch_item where news_id = %d"),m_id);

		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			watches.push_back((ULONG_PTR)q.getIntField(0));
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return watches.size();

}

void CNewsItem::Delete()
{
	CString sql;
	sql.Format(_T("delete from news_item where news_id=%d"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

void CNewsItem::ExportRss(MSXML2::IXMLDOMElementPtr& spItem)
{
	MSXML2::IXMLDOMDocumentPtr spDoc = spItem->ownerDocument;
	AddElement(spDoc,spItem,_T("title"), m_title);
	AddElement(spDoc,spItem,_T("link"),  m_url);
	AddElementCData(spDoc,spItem,_T("description"),  m_description);

	if(m_author.GetLength())
		AddElement(spDoc,spItem,_T("author"),  m_author);
	if(m_guid.GetLength())
		AddElement(spDoc,spItem,_T("guid"),  m_guid);
	if(m_date != 0)
		AddElement(spDoc,spItem,_T("pubDate"), CGMTimeHelper::FormatRFC822Date(m_date));
}

MSXML2::IXMLDOMDocumentPtr CNewsItem::GenerateFeedXml()
{
	MSXML2::IXMLDOMDocumentPtr spDoc(_T("MSXML2.DOMDocument.3.0")); 
	MSXML2::IXMLDOMElementPtr rss = spDoc->createElement("rss");
	rss->setAttribute("rss","2.0");
	spDoc->appendChild(rss);

	MSXML2::IXMLDOMElementPtr channelNode = spDoc->createElement("channel");
	NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(m_feedID);
	feed->ExportRss(channelNode);
	rss->appendChild(channelNode);

	MSXML2::IXMLDOMElementPtr itemNode = spDoc->createElement("item");
	ExportRss(itemNode);
	channelNode->appendChild(itemNode);
	
	return spDoc;
}


void CNewsItem::MarkItemsRead(std::vector<ULONG_PTR>& unreadIDs)
{
	ATLASSERT(!unreadIDs.empty());

	CContructIDList idList;
	idList = std::for_each(unreadIDs.begin(), unreadIDs.end(), idList);

	CString sql;
	sql.Format(_T("update news_item set readtime=1 where news_id in (%s) and readtime is null;"),
		(LPCTSTR)idList.GetIDList());

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	CNewsItemCache::MarkAllRead(unreadIDs);
}

int CNewsItem::GetNumOfAllItems()
{
	int n = 0;

	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql;
		sql.Format(_T("select count(*) from news_item where item_guid>'';")); // add where clause to force use index
		n = db.execScalar(sql);
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return n;
}

bool CNewsItem::RemoveDangerousTags()
{
#ifndef DEBUG
	// remove javascripts etc
	if(m_description.GetLength())
	{
		CRxUtil<CAtlRECharTraits> myRxUtil;
		// remove comments
		myRxUtil.s(m_description, _T("<\\!--(.*?|\\n*?)-->"),_T(" "),true, false, false);
		// remove some tags
		myRxUtil.s(m_description, _T("<(\\b)*?(embed)|(script)|(object)|(iframe)|(meta)|(frame)|(frameset)|(link)(.*?|\\n*?)>"),_T(" "),true, false, false);
		// remove attributes
		myRxUtil.s(m_description, _T("\\b+?(style)|(onabort)|(onblur)|(onchange)|(onclick)|(ondblclick)|(onfocus)|(onkeydown)|(onkeypress)|(onkeyup)|(onload)|(onmousedown)|(onmousemove)|(onmouseover)|(onmouseout)|(onmouseup)|(onreset)|(onselect)|(onsubmit)|(onunload)\\b*?="),_T(" xxxxx="),true, false, false);
		// remove style tag
		myRxUtil.s(m_description, _T("<(\\b)*?style(.*?|\\n*?)>(.*?|\\n*?)<(\\b)*?/(\\b)*?style(\\b)*?>"),_T(" "),true, false, false);
	}
#endif

	return true;
}

void CNewsItem::BatchLoadWatchIDs(NewsItemVector& newsItems)
{
	std::map<ULONG_PTR, NewsItemPtr> mapItems;
	CString ids;

	// find all items that haven't checked for watches
	for(NewsItemVector::iterator it = newsItems.begin(); it!=newsItems.end(); ++it)
	{
		NewsItemPtr& item = *it;
		
		if(!item->m_bWatchChecked)
		{
			item->m_vectWatches.clear();
			mapItems.insert(std::map<ULONG_PTR, NewsItemPtr>::value_type(item->m_id, item));
			ids.AppendFormat(_T("%d,"), item->m_id);
		}
	}

	if(ids.IsEmpty())
		return; // all items checked already. nothing to do.

	// load watch ids
	ids.TrimRight(_T(','));
	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql;
		sql.Format(_T("select news_id, watch_id from watch_item where news_id in (%s)"),(LPCTSTR)ids);

		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			NewsItemPtr& item = mapItems[(ULONG_PTR)q.getIntField(0)];
			if(item != NULL)
			{
				item->m_vectWatches.push_back((ULONG_PTR)q.getIntField(0));
			}
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	// figure out colors
	for(std::map<ULONG_PTR, NewsItemPtr>::iterator it = mapItems.begin(); it!=mapItems.end(); ++it)
	{
		NewsItemPtr item = it->second;
		item->CalculateColors();
	}
}


bool CNewsItem::CalculateColors()
{
	if(m_vectWatches.empty())
	{
		m_bWatchChecked = true;
		return true;
	}

	NewsWatchPtr watch = CNewsWatchCache::GetHighestNewsWatch(m_vectWatches);
	if(watch)
	{
		m_textColor = watch->m_txtColor;
		m_bkColor =  watch->m_bkColor;
		m_bWatchChecked = true;
		return true;
	}
	else
	{
		return false;
	}
}

void CNewsItem::SetTrackComment(bool bTrack)
{
	m_bTrackComments = bTrack;
	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql;
		sql.Format(_T("update news_item set commentTrack = %d where news_id = %d"),
			bTrack? 1 : 0,
			m_id);

		db.execDML(sql);
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}
