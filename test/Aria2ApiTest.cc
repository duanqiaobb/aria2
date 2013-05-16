#include "aria2api.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class Aria2ApiTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Aria2ApiTest);
  CPPUNIT_TEST(testAddUri);
  CPPUNIT_TEST(testAddMetalink);
  CPPUNIT_TEST(testAddTorrent);
  CPPUNIT_TEST(testRemovePause);
  CPPUNIT_TEST(testChangePosition);
  CPPUNIT_TEST(testChangeOption);
  CPPUNIT_TEST_SUITE_END();

  Session* session_;
public:
  void setUp()
  {
    SessionConfig config;
    KeyVals options;
    session_ = sessionNew(options, config);
  }

  void tearDown()
  {
    sessionFinal(session_);
  }

  void testAddUri();
  void testAddMetalink();
  void testAddTorrent();
  void testRemovePause();
  void testChangePosition();
  void testChangeOption();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Aria2ApiTest);

void Aria2ApiTest::testAddUri()
{
  A2Gid gid;
  std::vector<std::string> uris(1);
  KeyVals options;
  uris[0] = "http://localhost/1";
  CPPUNIT_ASSERT_EQUAL(0, addUri(session_, &gid, uris, options));
  CPPUNIT_ASSERT(!isNull(gid));

  DownloadHandle* hd = getDownloadHandle(session_, gid);
  CPPUNIT_ASSERT(hd);
  CPPUNIT_ASSERT_EQUAL(1, hd->getNumFiles());
  FileData file = hd->getFile(1);
  CPPUNIT_ASSERT_EQUAL((size_t)1, file.uris.size());
  CPPUNIT_ASSERT_EQUAL(uris[0], file.uris[0].uri);
  deleteDownloadHandle(hd);

  options.push_back(KeyVals::value_type("file-allocation", "foo"));
  CPPUNIT_ASSERT_EQUAL(-1, addUri(session_, &gid, uris, options));
}

void Aria2ApiTest::testAddMetalink()
{
  std::string metalinkPath = A2_TEST_DIR"/metalink4.xml";
  std::vector<A2Gid> gids;
  KeyVals options;
#ifdef ENABLE_METALINK
  CPPUNIT_ASSERT_EQUAL(0, addMetalink(session_, &gids, metalinkPath, options));
  CPPUNIT_ASSERT_EQUAL((size_t)2, gids.size());

  gids.clear();
  options.push_back(KeyVals::value_type("file-allocation", "foo"));
  CPPUNIT_ASSERT_EQUAL(-1, addMetalink(session_, &gids, metalinkPath,
                                       options));
#else // !ENABLE_METALINK
  CPPUNIT_ASSERT_EQUAL(-1, addMetalink(session_, &gids, metalinkPath,
                                       options));
#endif // !ENABLE_METALINK
}

void Aria2ApiTest::testAddTorrent()
{
  std::string torrentPath = A2_TEST_DIR"/test.torrent";
  A2Gid gid;
  KeyVals options;
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL(0, addTorrent(session_, &gid, torrentPath, options));
  CPPUNIT_ASSERT(!isNull(gid));

  options.push_back(KeyVals::value_type("file-allocation", "foo"));
  CPPUNIT_ASSERT_EQUAL(-1, addTorrent(session_, &gid, torrentPath, options));
#else // !ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL(-1, addTorrent(session_, &gid, torrentPath, options));
#endif // !ENABLE_BITTORRENT
}

void Aria2ApiTest::testRemovePause()
{
  A2Gid gid;
  std::vector<std::string> uris(1);
  KeyVals options;
  uris[0] = "http://localhost/1";
  CPPUNIT_ASSERT_EQUAL(0, addUri(session_, &gid, uris, options));

  DownloadHandle* hd = getDownloadHandle(session_, gid);
  CPPUNIT_ASSERT(hd);
  CPPUNIT_ASSERT_EQUAL(DOWNLOAD_WAITING, hd->getStatus());
  deleteDownloadHandle(hd);

  CPPUNIT_ASSERT_EQUAL(-1, pauseDownload(session_, (A2Gid)0));
  CPPUNIT_ASSERT_EQUAL(0, pauseDownload(session_, gid));
  hd = getDownloadHandle(session_, gid);
  CPPUNIT_ASSERT(hd);
  CPPUNIT_ASSERT_EQUAL(DOWNLOAD_PAUSED, hd->getStatus());
  deleteDownloadHandle(hd);

  CPPUNIT_ASSERT_EQUAL(-1, unpauseDownload(session_, (A2Gid)0));
  CPPUNIT_ASSERT_EQUAL(0, unpauseDownload(session_, gid));
  hd = getDownloadHandle(session_, gid);
  CPPUNIT_ASSERT(hd);
  CPPUNIT_ASSERT_EQUAL(DOWNLOAD_WAITING, hd->getStatus());
  deleteDownloadHandle(hd);

  CPPUNIT_ASSERT_EQUAL(-1, removeDownload(session_, (A2Gid)0));
  CPPUNIT_ASSERT_EQUAL(0, removeDownload(session_, gid));
  hd = getDownloadHandle(session_, gid);
  CPPUNIT_ASSERT(!hd);
}

void Aria2ApiTest::testChangePosition()
{
  int N = 10;
  std::vector<A2Gid> gids(N);
  std::vector<std::string> uris(1);
  KeyVals options;
  uris[0] = "http://localhost/";
  for(int i = 0; i < N; ++i) {
    CPPUNIT_ASSERT_EQUAL(0, addUri(session_, &gids[i], uris, options));
  }
  CPPUNIT_ASSERT_EQUAL(-1, changePosition(session_, (A2Gid)0, -2,
                                          OFFSET_MODE_CUR));
  CPPUNIT_ASSERT_EQUAL(2, changePosition(session_, gids[4], -2,
                                         OFFSET_MODE_CUR));

  CPPUNIT_ASSERT_EQUAL(5, changePosition(session_, gids[4], 5,
                                         OFFSET_MODE_SET));

  CPPUNIT_ASSERT_EQUAL(7, changePosition(session_, gids[4], -2,
                                         OFFSET_MODE_END));

}

void Aria2ApiTest::testChangeOption()
{
  A2Gid gid;
  std::vector<std::string> uris(1);
  KeyVals options;
  uris[0] = "http://localhost/1";
  options.push_back(KeyVals::value_type("dir", "mydownload"));
  CPPUNIT_ASSERT_EQUAL(0, addUri(session_, &gid, uris, options));

  DownloadHandle* hd = getDownloadHandle(session_, gid);
  CPPUNIT_ASSERT(hd);
  CPPUNIT_ASSERT_EQUAL(1, hd->getNumFiles());
  FileData file = hd->getFile(1);
  CPPUNIT_ASSERT_EQUAL((size_t)1, file.uris.size());
  CPPUNIT_ASSERT_EQUAL(uris[0], file.uris[0].uri);

  CPPUNIT_ASSERT_EQUAL(std::string("mydownload"), hd->getOption("dir"));
  CPPUNIT_ASSERT(hd->getOption("unknown").empty());
  KeyVals retopts = hd->getOption();
  CPPUNIT_ASSERT(std::find(retopts.begin(), retopts.end(),
                           KeyVals::value_type("dir", "mydownload"))
                 != retopts.end());
  deleteDownloadHandle(hd);
  // failure with null gid
  CPPUNIT_ASSERT_EQUAL(-1, changeOption(session_, (A2Gid)0, options));
  // change option
  options.clear();
  options.push_back(KeyVals::value_type("dir", "newlocation"));
  CPPUNIT_ASSERT_EQUAL(0, changeOption(session_, gid, options));
  hd = getDownloadHandle(session_, gid);
  CPPUNIT_ASSERT_EQUAL(std::string("newlocation"), hd->getOption("dir"));
  deleteDownloadHandle(hd);
  // failure with bad option value
  options.clear();
  options.push_back(KeyVals::value_type("file-allocation", "foo"));
  CPPUNIT_ASSERT_EQUAL(-1, changeOption(session_, gid, options));
}


} // namespace aria2