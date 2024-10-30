#include <gtest/gtest.h>
#include <net/ancillarycat/waver/waver.hpp>

extern std::string vcd_string;
TEST(dummy, test) {
  ASSERT_TRUE(true);
  ASSERT_EQ(true, !!true);
}

TEST(waver, test) {
  using namespace net::ancillarycat::waver;
  auto vcd = net::ancillarycat::waver::value_change_dump::parse(vcd_string);

  auto json = vcd->as_json();
  ASSERT_TRUE(json.is_object());
  std::println("{}", json.dump(2));
}

std::string vcd_string = R"(
$version Generated by VerilatedVcd $end
$timescale 1s $end
 $scope module TOP $end
  $var wire 4 ) lhs [3:0] $end
  $var wire 4 * rhs [3:0] $end
  $var wire 3 + op [2:0] $end
  $var wire 4 , out [3:0] $end
  $var wire 1 - zero $end
  $var wire 1 . overflow $end
  $var wire 1 / carry $end
  $scope module ALU4 $end
   $var wire 4 ) lhs [3:0] $end
   $var wire 4 * rhs [3:0] $end
   $var wire 3 + op [2:0] $end
   $var wire 4 , out [3:0] $end
   $var wire 1 - zero $end
   $var wire 1 . overflow $end
   $var wire 1 / carry $end
   $var wire 8 0 sel [7:0] $end
   $var wire 4 1 cla4_out [3:0] $end
   $var wire 1 # op_add $end
   $var wire 1 2 op_sub $end
   $var wire 1 3 op_not_lhs $end
   $var wire 4 4 not_lhs_out [3:0] $end
   $var wire 1 5 op_and $end
   $var wire 4 6 and_out [3:0] $end
   $var wire 1 7 op_or $end
   $var wire 4 8 or_out [3:0] $end
   $var wire 1 9 op_xor $end
   $var wire 4 : xor_out [3:0] $end
   $var wire 1 ; op_lhs_eq $end
   $var wire 1 $ is_eq $end
   $var wire 1 < op_lhs_less $end
   $var wire 1 = is_rhs_bigger $end
   $scope module cla4 $end
    $var wire 4 ) lhs [3:0] $end
    $var wire 4 * rhs [3:0] $end
    $var wire 1 % carry_in $end
    $var wire 4 1 out [3:0] $end
    $var wire 1 / carry_out $end
    $var wire 1 . overflow $end
    $var wire 4 & rhs_inv [3:0] $end
    $var wire 4 ' gen [3:0] $end
    $var wire 4 ( prop [3:0] $end
    $var wire 5 > carries [4:0] $end
   $upscope $end
   $scope module cmp4_identical $end
    $var wire 4 ) lhs [3:0] $end
    $var wire 4 * rhs [3:0] $end
    $var wire 1 $ out $end
   $upscope $end
   $scope module cmp4_rhs $end
    $var wire 4 ) lhs [3:0] $end
    $var wire 4 * rhs [3:0] $end
    $var wire 1 = out $end
   $upscope $end
   $scope module d38 $end
    $var wire 3 + in [2:0] $end
    $var wire 1 ? enable_signal $end
    $var wire 8 0 out [7:0] $end
   $upscope $end
  $upscope $end
 $upscope $end
$enddefinitions $end


#1
1#
0$
0%
b0010 &
b0010 '
b1011 (
b1011 )
b0010 *
b000 +
b1111 ,
0-
0.
0/
b00000001 0
b1111 1
02
03
b0100 4
05
b0010 6
07
b1011 8
09
b1001 :
0;
0<
1=
b00100 >
1?
#2
0#
1%
b1101 &
b1001 '
b1111 (
b001 +
b0000 ,
1-
1/
b00000010 0
b0000 1
12
b11111 >
#3
b010 +
b0100 ,
0-
b00000100 0
02
13
#4
)";