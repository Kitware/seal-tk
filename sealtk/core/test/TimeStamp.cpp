/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/TimeStamp.hpp>

#include <QObject>
#include <QtTest>

Q_DECLARE_METATYPE(sealtk::core::TimeStamp)

namespace sealtk
{

namespace core
{

namespace test
{

// ============================================================================
struct TimeStampTestData
{
  char const* name;
  TimeStamp t1;
  TimeStamp t2;
  bool eq;
  bool ne;
  bool ge;
  bool le;
  bool gt;
  bool lt;
};

// ============================================================================
class TestTimeStamp : public QObject
{
  Q_OBJECT

public:
  TestTimeStamp();

private slots:
  void eq();
  void eq_data();
  void ne();
  void ne_data();
  void ge();
  void ge_data();
  void le();
  void le_data();
  void gt();
  void gt_data();
  void lt();
  void lt_data();

private:
  QVector<TimeStampTestData> data;
};

// ----------------------------------------------------------------------------
TestTimeStamp::TestTimeStamp()
{
  TimeStampTestData d;

  d = TimeStampTestData{};
  d.name = "1";
  d.eq = false;
  d.ne = true;
  d.ge = false;
  d.le = false;
  d.gt = false;
  d.lt = false;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "2";
  d.t1.set_time_usec(500);
  d.t1.set_time_domain(0);
  d.t1.set_frame(10);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(1000);
  d.t2.set_time_domain(0);
  d.t2.set_frame(5);
  d.t2.set_frame_domain(0);
  d.eq = false;
  d.ne = true;
  d.ge = false;
  d.le = true;
  d.gt = false;
  d.lt = true;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "3";
  d.t1.set_time_usec(500);
  d.t1.set_time_domain(0);
  d.t1.set_frame(10);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(500);
  d.t2.set_time_domain(0);
  d.t2.set_frame(10);
  d.t2.set_frame_domain(0);
  d.eq = true;
  d.ne = false;
  d.ge = true;
  d.le = true;
  d.gt = false;
  d.lt = false;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "4";
  d.t1.set_time_usec(1000);
  d.t1.set_time_domain(0);
  d.t1.set_frame(5);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(500);
  d.t2.set_time_domain(0);
  d.t2.set_frame(10);
  d.t2.set_frame_domain(0);
  d.eq = false;
  d.ne = true;
  d.ge = true;
  d.le = false;
  d.gt = true;
  d.lt = false;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "5";
  d.t1.set_time_usec(500);
  d.t1.set_time_domain(0);
  d.t1.set_frame(5);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(500);
  d.t2.set_time_domain(1);
  d.t2.set_frame(10);
  d.t2.set_frame_domain(1);
  d.eq = false;
  d.ne = true;
  d.ge = false;
  d.le = false;
  d.gt = false;
  d.lt = false;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "6";
  d.t1.set_time_usec(500);
  d.t1.set_time_domain(0);
  d.t1.set_frame(5);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(500);
  d.t2.set_time_domain(1);
  d.t2.set_frame(10);
  d.t2.set_frame_domain(0);
  d.eq = false;
  d.ne = true;
  d.ge = false;
  d.le = true;
  d.gt = false;
  d.lt = true;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "7";
  d.t1.set_time_usec(500);
  d.t1.set_time_domain(0);
  d.t1.set_frame(5);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(500);
  d.t2.set_time_domain(1);
  d.t2.set_frame(5);
  d.t2.set_frame_domain(0);
  d.eq = true;
  d.ne = false;
  d.ge = true;
  d.le = true;
  d.gt = false;
  d.lt = false;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "8";
  d.t1.set_time_usec(500);
  d.t1.set_time_domain(0);
  d.t1.set_frame(10);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(500);
  d.t2.set_time_domain(1);
  d.t2.set_frame(5);
  d.t2.set_frame_domain(0);
  d.eq = false;
  d.ne = true;
  d.ge = true;
  d.le = false;
  d.gt = true;
  d.lt = false;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "9";
  d.t1.set_time_usec(500);
  d.t1.set_time_domain(0);
  d.t1.set_frame(5);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(500);
  d.t2.set_time_domain(1);
  d.t2.set_frame(5);
  d.t2.set_frame_domain(1);
  d.eq = false;
  d.ne = true;
  d.ge = false;
  d.le = false;
  d.gt = false;
  d.lt = false;
  this->data.append(d);

  d = TimeStampTestData{};
  d.name = "10";
  d.t1.set_time_usec(500);
  d.t1.set_time_domain(0);
  d.t1.set_frame(5);
  d.t1.set_frame_domain(0);
  d.t2.set_time_usec(500);
  d.t2.set_time_domain(0);
  d.t2.set_frame(10);
  d.t2.set_frame_domain(1);
  d.eq = true;
  d.ne = false;
  d.ge = true;
  d.le = true;
  d.gt = false;
  d.lt = false;
  this->data.append(d);
}

// ----------------------------------------------------------------------------
#define COMPARE(NAME, OP)                                                     \
  void TestTimeStamp::NAME()                                                  \
  {                                                                           \
    QFETCH(TimeStamp, t1);                                                    \
    QFETCH(TimeStamp, t2);                                                    \
    QFETCH(bool, expected);                                                   \
                                                                              \
    QCOMPARE((t1 OP t2), expected);                                           \
  }                                                                           \
                                                                              \
  void TestTimeStamp::NAME##_data()                                           \
  {                                                                           \
    QTest::addColumn<TimeStamp>("t1");                                        \
    QTest::addColumn<TimeStamp>("t2");                                        \
    QTest::addColumn<bool>("expected");                                       \
                                                                              \
    for (auto const& d : this->data)                                          \
    {                                                                         \
      QTest::newRow(d.name) << d.t1 << d.t2 << d.NAME;                        \
    }                                                                         \
  }

COMPARE(eq, ==)
COMPARE(ne, !=)
COMPARE(ge, >=)
COMPARE(le, <=)
COMPARE(gt, >)
COMPARE(lt, <)

} // namespace test

} // namespace core

} // namespace sealtk

Q_DECLARE_METATYPE(sealtk::core::test::TimeStampTestData)

// ----------------------------------------------------------------------------
QTEST_MAIN(sealtk::core::test::TestTimeStamp)
#include "TimeStamp.moc"
