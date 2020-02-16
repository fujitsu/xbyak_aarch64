tbl = [
  "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE",
]

for x in tbl:
  low = x.lower()
  print(f'void b{low}(const Label &label) {{ b({x}, label); }}')
  print(f'void b{low}(int64_t label) {{ b({x}, label); }}')
