tbl = [
  "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE",
]

sveTbl = [
  ("eq", "none"), ("ne", "any"), ("cs", "nlast"), ("cc", "last"), ("mi", "first"), ("pl", "nfrst"), ("hi", "pmore"), ("ls", "plast"), ("ge", "tcont"), ("lt", "tstop")
]

for x in tbl:
  low = x.lower()
  print(f'void b{low}(const Label &label) {{ b({x}, label); }}')
  print(f'void b{low}(int64_t label) {{ b({x}, label); }}')

for (a64,sve) in sveTbl:
  print(f'void b_{sve}(const Label& label) {{ b{a64}(label); }}')
  print(f'void b_{sve}(int64_t label) {{ b{a64}(label); }}')
