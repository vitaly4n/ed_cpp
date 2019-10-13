#include <cstddef>
#include <cstdint>

struct Nucleotide
{
  char Symbol;       // A, T, G, C
  size_t Position;   // 0 - 3.2e9
  int ChromosomeNum; // 0 - 46
  int GeneNum;       // 1 - 25e3
  bool IsMarked;     // flag
  char ServiceInfo;  // 0 - 255
};

struct CompactNucleotide
{
  uint32_t Position;
  uint32_t Symbol : 2;
  uint32_t ChromosomeNum : 6;
  uint32_t GenNum : 15;
  uint32_t IsMarked : 1;
  uint32_t ServiceInfo : 8;
};

static_assert(sizeof(CompactNucleotide) <= 8, "Too large CompacctNucleotide!");

inline bool
operator==(const Nucleotide& lhs, const Nucleotide& rhs)
{
  return (lhs.Symbol == rhs.Symbol) && (lhs.Position == rhs.Position) && (lhs.ChromosomeNum == rhs.ChromosomeNum) &&
         (lhs.GeneNum == rhs.GeneNum) && (lhs.IsMarked == rhs.IsMarked) && (lhs.ServiceInfo == rhs.ServiceInfo);
}

inline uint32_t
SymbolToIdx(char symbol)
{
  switch (symbol) {
    case 'A':
      return 0;
    case 'T':
      return 1;
    case 'G':
      return 2;
    case 'C':
      return 3;
  }
  return -1;
}

inline char
IdxToSymbol(uint32_t idx)
{
  switch (idx) {
    case 0:
      return 'A';
    case 1:
      return 'T';
    case 2:
      return 'G';
    case 3:
      return 'C';
  }
  return -1;
}

inline CompactNucleotide
Compress(const Nucleotide& n)
{
  CompactNucleotide cn;
  cn.Position = static_cast<uint32_t>(n.Position);
  cn.Symbol = SymbolToIdx(n.Symbol);
  cn.ChromosomeNum = static_cast<uint32_t>(n.ChromosomeNum);
  cn.GenNum = static_cast<uint32_t>(n.GeneNum);
  cn.IsMarked = static_cast<uint32_t>(n.IsMarked);
  cn.ServiceInfo = static_cast<uint32_t>(n.ServiceInfo);
  return cn;
}

inline Nucleotide
Decompress(const CompactNucleotide& cn)
{
  Nucleotide n;
  n.Position = static_cast<size_t>(cn.Position);
  n.Symbol = IdxToSymbol(cn.Symbol);
  n.ChromosomeNum = static_cast<int>(cn.ChromosomeNum);
  n.GeneNum = static_cast<int>(cn.GenNum);
  n.IsMarked = static_cast<bool>(cn.IsMarked);
  n.ServiceInfo = static_cast<char>(cn.ServiceInfo);
  return n;
}
