void gen() {
ttest(x0); dump();
ttest(x1); dump();
ttest(x2); dump();
ttest(x4); dump();
ttest(x8); dump();
ttest(x16); dump();
ttest(x30); dump();
ttest(xzr); dump();
tstart(x0); dump();
tstart(x1); dump();
tstart(x2); dump();
tstart(x4); dump();
tstart(x8); dump();
tstart(x16); dump();
tstart(x30); dump();
tstart(xzr); dump();
tcancel(1); dump();
tcancel((1<<4)); dump();
tcancel((1<<8)); dump();
tcancel((1<<12)); dump();
tcancel((0xffff)); dump();
}
