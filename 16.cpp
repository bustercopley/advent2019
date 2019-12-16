#include "precompiled.h"
#include "intcode.h"
#include "rectangle.h"
#include <set>

std::string fft(std::string s) {
  std::string result;
  result.resize(std::size(s));
  int patdef[] = {0, 1, 0, -1};
  for (int n = 0; (std::size_t)n != std::size(s); ++n) {
    int d = 0;
    for (int i = 0; (std::size_t)i != std::size(s); ++i) {
      int pattern_digit = patdef[((i + 1) / (n + 1)) % 4];
      int c = (int)(s[i] - '0');
      d = d + pattern_digit * c;
    }
    d = std::abs(d) % 10;
    result[n] = (char)('0' + d);
  }
  return result;
}

void part_one(std::string s, int rounds) {
  for (int i = 0; i != rounds; ++i) {
    s = fft(s);
  }
  std::cout << s << std::endl;
}

// Thanks bluepichu:

// The key observation for part 2 is that if your sequence has length n and you
// want to compute the value of the next phase at index i with i > n/2, then
// it's just the sum of all of the elements with index at least i. Therefore, we
// only need to compute the sequence for indices higher than the given offset at
// every phase, and we can do so in linear time.

void part_two(std::string s) {
  int offset;
  std::from_chars(&s[0], &s[7], offset);
  offset -= 5000 * std::size(s);

  std::string t;
  {
    std::ostringstream ostr;
    for (int i = 0; i != 5000; ++i) {
      ostr << s;
    }
    t = ostr.str();
  }

  for (int i = 0; i != 100; ++i) {
    int a = 0;
    for (int i = 0; (std::size_t)i != std::size(t); ++i) {
      int j = std::size(t) - i - 1;
      a = (a + (int)(t[j] - '0')) % 10;
      t[j] = '0' + a;
    }
  }

  std::cout << std::string (&t[offset], &t[offset + 8]) << std::endl;
}

int CALLBACK _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
  try {
    part_one("12345678", 4);
    part_one("80871224585914546619083218645595", 100);
    part_one(
      "597967370476643225434885050821479669972464655808057915784174627887807404"
      "844096256746766609475415714489100070028214540689456539114861408231682339"
      "152852290753740008880299778003416635860466220036207703617382700142467309"
      "360464718318043082631773317234607877124235874537258400422345502999912380"
      "293072053489589927940244022537473406303789446723008746914786318466178612"
      "550157702986994072543118894845085458612644498789846243303242282780573773"
      "130298025053762601969042137462818302143523376220134730192450818348547812"
      "775657065457204922826164889507312919743286722526576313537654969791428304"
      "598896824753976866519233180156276941768936439698646892576200269166153053"
      "97",
      100);

    std::cout << "Part two" << std::endl;
    part_two("03036732577212944063491565474664");
    std::cout << "Here goes" << std::endl;
    part_two(
      "597967370476643225434885050821479669972464655808057915784174627887807404"
      "844096256746766609475415714489100070028214540689456539114861408231682339"
      "152852290753740008880299778003416635860466220036207703617382700142467309"
      "360464718318043082631773317234607877124235874537258400422345502999912380"
      "293072053489589927940244022537473406303789446723008746914786318466178612"
      "550157702986994072543118894845085458612644498789846243303242282780573773"
      "130298025053762601969042137462818302143523376220134730192450818348547812"
      "775657065457204922826164889507312919743286722526576313537654969791428304"
      "598896824753976866519233180156276941768936439698646892576200269166153053"
      "97");

    return 0;
  } catch (const char *e) {
    std::cout << e << std::endl;
    return 1;
  }
}
