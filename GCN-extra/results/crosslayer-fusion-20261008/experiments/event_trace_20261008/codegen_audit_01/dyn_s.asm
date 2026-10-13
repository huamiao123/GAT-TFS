0000000000468670 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)>:
  468670:	55                   	push   %rbp
  468671:	41 57                	push   %r15
  468673:	41 56                	push   %r14
  468675:	41 55                	push   %r13
  468677:	41 54                	push   %r12
  468679:	53                   	push   %rbx
  46867a:	48 89 54 24 d8       	mov    %rdx,-0x28(%rsp)
  46867f:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  468684:	44 0f b6 5c 24 50    	movzbl 0x50(%rsp),%r11d
  46868a:	8b 6c 24 48          	mov    0x48(%rsp),%ebp
  46868e:	44 8b 74 24 40       	mov    0x40(%rsp),%r14d
  468693:	44 8b 7c 24 38       	mov    0x38(%rsp),%r15d
  468698:	49 63 c0             	movslq %r8d,%rax
  46869b:	48 89 44 24 f0       	mov    %rax,-0x10(%rsp)
  4686a0:	8b 04 81             	mov    (%rcx,%rax,4),%eax
  4686a3:	39 e8                	cmp    %ebp,%eax
  4686a5:	0f 43 c5             	cmovae %ebp,%eax
  4686a8:	44 39 f0             	cmp    %r14d,%eax
  4686ab:	44 89 f2             	mov    %r14d,%edx
  4686ae:	89 44 24 ec          	mov    %eax,-0x14(%rsp)
  4686b2:	0f 42 d0             	cmovb  %eax,%edx
  4686b5:	44 39 fa             	cmp    %r15d,%edx
  4686b8:	0f 97 c0             	seta   %al
  4686bb:	45 89 dc             	mov    %r11d,%r12d
  4686be:	41 08 c4             	or     %al,%r12b
  4686c1:	45 85 c9             	test   %r9d,%r9d
  4686c4:	0f 88 89 00 00 00    	js     468753 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xe3>
  4686ca:	45 89 ca             	mov    %r9d,%r10d
  4686cd:	42 8b 0c 91          	mov    (%rcx,%r10,4),%ecx
  4686d1:	39 e9                	cmp    %ebp,%ecx
  4686d3:	0f 42 e9             	cmovb  %ecx,%ebp
  4686d6:	44 39 f5             	cmp    %r14d,%ebp
  4686d9:	44 89 f1             	mov    %r14d,%ecx
  4686dc:	41 89 ed             	mov    %ebp,%r13d
  4686df:	0f 42 cd             	cmovb  %ebp,%ecx
  4686e2:	44 39 f9             	cmp    %r15d,%ecx
  4686e5:	40 0f 97 c5          	seta   %bpl
  4686e9:	44 08 dd             	or     %r11b,%bpl
  4686ec:	45 84 e4             	test   %r12b,%r12b
  4686ef:	0f 84 bc 00 00 00    	je     4687b1 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x141>
  4686f5:	45 84 db             	test   %r11b,%r11b
  4686f8:	0f 84 c0 00 00 00    	je     4687be <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x14e>
  4686fe:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  468703:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  468707:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  46870b:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  46870f:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  468713:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  468717:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  46871b:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  46871f:	40 84 ed             	test   %bpl,%bpl
  468722:	0f 85 e8 00 00 00    	jne    468810 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x1a0>
  468728:	44 89 44 24 e4       	mov    %r8d,-0x1c(%rsp)
  46872d:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  468732:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  468739:	00 
  46873a:	89 c5                	mov    %eax,%ebp
  46873c:	45 31 db             	xor    %r11d,%r11d
  46873f:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  468745:	40 84 ed             	test   %bpl,%bpl
  468748:	0f 85 69 01 00 00    	jne    4688b7 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x247>
  46874e:	e9 80 01 00 00       	jmpq   4688d3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x263>
  468753:	45 84 e4             	test   %r12b,%r12b
  468756:	0f 84 57 0a 00 00    	je     4691b3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  46875c:	45 84 db             	test   %r11b,%r11b
  46875f:	0f 84 bd 01 00 00    	je     468922 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x2b2>
  468765:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  46876a:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  46876e:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  468772:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  468776:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  46877a:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  46877e:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  468782:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  468786:	44 89 44 24 e4       	mov    %r8d,-0x1c(%rsp)
  46878b:	44 39 fa             	cmp    %r15d,%edx
  46878e:	0f 87 e4 01 00 00    	ja     468978 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x308>
  468794:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  468799:	45 31 ed             	xor    %r13d,%r13d
  46879c:	45 31 e4             	xor    %r12d,%r12d
  46879f:	31 c9                	xor    %ecx,%ecx
  4687a1:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  4687a8:	00 
  4687a9:	45 31 db             	xor    %r11d,%r11d
  4687ac:	e9 3e 01 00 00       	jmpq   4688ef <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x27f>
  4687b1:	40 84 ed             	test   %bpl,%bpl
  4687b4:	0f 84 f9 09 00 00    	je     4691b3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  4687ba:	31 ed                	xor    %ebp,%ebp
  4687bc:	eb 54                	jmp    468812 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x1a2>
  4687be:	45 89 c6             	mov    %r8d,%r14d
  4687c1:	41 c1 e6 07          	shl    $0x7,%r14d
  4687c5:	4d 63 f6             	movslq %r14d,%r14
  4687c8:	62 b1 7c 48 10 44 b3 	vmovups 0x1c0(%rbx,%r14,4),%zmm0
  4687cf:	07 
  4687d0:	62 b1 7c 48 10 4c b3 	vmovups 0x180(%rbx,%r14,4),%zmm1
  4687d7:	06 
  4687d8:	62 b1 7c 48 10 54 b3 	vmovups 0x140(%rbx,%r14,4),%zmm2
  4687df:	05 
  4687e0:	62 b1 7c 48 10 5c b3 	vmovups 0x100(%rbx,%r14,4),%zmm3
  4687e7:	04 
  4687e8:	62 31 7c 48 10 04 b3 	vmovups (%rbx,%r14,4),%zmm8
  4687ef:	62 b1 7c 48 10 7c b3 	vmovups 0x40(%rbx,%r14,4),%zmm7
  4687f6:	01 
  4687f7:	62 b1 7c 48 10 74 b3 	vmovups 0x80(%rbx,%r14,4),%zmm6
  4687fe:	02 
  4687ff:	62 b1 7c 48 10 64 b3 	vmovups 0xc0(%rbx,%r14,4),%zmm4
  468806:	03 
  468807:	40 84 ed             	test   %bpl,%bpl
  46880a:	0f 84 18 ff ff ff    	je     468728 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb8>
  468810:	89 c5                	mov    %eax,%ebp
  468812:	41 b6 01             	mov    $0x1,%r14b
  468815:	44 89 74 24 d4       	mov    %r14d,-0x2c(%rsp)
  46881a:	45 84 db             	test   %r11b,%r11b
  46881d:	44 89 44 24 e4       	mov    %r8d,-0x1c(%rsp)
  468822:	44 88 64 24 d3       	mov    %r12b,-0x2d(%rsp)
  468827:	74 37                	je     468860 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x1f0>
  468829:	c4 41 10 57 ed       	vxorps %xmm13,%xmm13,%xmm13
  46882e:	c4 41 00 57 ff       	vxorps %xmm15,%xmm15,%xmm15
  468833:	c4 41 08 57 f6       	vxorps %xmm14,%xmm14,%xmm14
  468838:	c4 41 18 57 e4       	vxorps %xmm12,%xmm12,%xmm12
  46883d:	c4 41 20 57 db       	vxorps %xmm11,%xmm11,%xmm11
  468842:	c4 41 28 57 d2       	vxorps %xmm10,%xmm10,%xmm10
  468847:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  46884c:	c5 d0 57 ed          	vxorps %xmm5,%xmm5,%xmm5
  468850:	45 31 db             	xor    %r11d,%r11d
  468853:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  468859:	40 84 ed             	test   %bpl,%bpl
  46885c:	75 59                	jne    4688b7 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x247>
  46885e:	eb 73                	jmp    4688d3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x263>
  468860:	45 89 cb             	mov    %r9d,%r11d
  468863:	41 c1 e3 07          	shl    $0x7,%r11d
  468867:	4d 63 db             	movslq %r11d,%r11
  46886a:	62 b1 7c 48 10 6c 9b 	vmovups 0x1c0(%rbx,%r11,4),%zmm5
  468871:	07 
  468872:	62 31 7c 48 10 4c 9b 	vmovups 0x180(%rbx,%r11,4),%zmm9
  468879:	06 
  46887a:	62 31 7c 48 10 54 9b 	vmovups 0x140(%rbx,%r11,4),%zmm10
  468881:	05 
  468882:	62 31 7c 48 10 5c 9b 	vmovups 0x100(%rbx,%r11,4),%zmm11
  468889:	04 
  46888a:	62 31 7c 48 10 2c 9b 	vmovups (%rbx,%r11,4),%zmm13
  468891:	62 31 7c 48 10 7c 9b 	vmovups 0x40(%rbx,%r11,4),%zmm15
  468898:	01 
  468899:	62 31 7c 48 10 74 9b 	vmovups 0x80(%rbx,%r11,4),%zmm14
  4688a0:	02 
  4688a1:	62 31 7c 48 10 64 9b 	vmovups 0xc0(%rbx,%r11,4),%zmm12
  4688a8:	03 
  4688a9:	45 31 db             	xor    %r11d,%r11d
  4688ac:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  4688b2:	40 84 ed             	test   %bpl,%bpl
  4688b5:	74 1c                	je     4688d3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x263>
  4688b7:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  4688bc:	4c 8b 74 24 f0       	mov    -0x10(%rsp),%r14
  4688c1:	47 8b 34 b0          	mov    (%r8,%r14,4),%r14d
  4688c5:	45 89 fc             	mov    %r15d,%r12d
  4688c8:	4d 01 f4             	add    %r14,%r12
  4688cb:	4c 8b 77 18          	mov    0x18(%rdi),%r14
  4688cf:	47 8b 24 a6          	mov    (%r14,%r12,4),%r12d
  4688d3:	44 39 f9             	cmp    %r15d,%ecx
  4688d6:	76 17                	jbe    4688ef <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x27f>
  4688d8:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  4688dd:	47 8b 14 90          	mov    (%r8,%r10,4),%r10d
  4688e1:	45 89 fb             	mov    %r15d,%r11d
  4688e4:	4d 01 d3             	add    %r10,%r11
  4688e7:	4c 8b 57 18          	mov    0x18(%rdi),%r10
  4688eb:	47 8b 1c 9a          	mov    (%r10,%r11,4),%r11d
  4688ef:	45 85 c9             	test   %r9d,%r9d
  4688f2:	40 0f 99 c5          	setns  %bpl
  4688f6:	44 39 f9             	cmp    %r15d,%ecx
  4688f9:	41 0f 97 c2          	seta   %r10b
  4688fd:	41 20 ea             	and    %bpl,%r10b
  468900:	44 39 fa             	cmp    %r15d,%edx
  468903:	77 11                	ja     468916 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x2a6>
  468905:	45 84 d2             	test   %r10b,%r10b
  468908:	75 0c                	jne    468916 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x2a6>
  46890a:	89 4c 24 e8          	mov    %ecx,-0x18(%rsp)
  46890e:	44 89 ed             	mov    %r13d,%ebp
  468911:	e9 58 04 00 00       	jmpq   468d6e <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6fe>
  468916:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  46891a:	44 89 ed             	mov    %r13d,%ebp
  46891d:	e9 89 00 00 00       	jmpq   4689ab <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x33b>
  468922:	44 89 c1             	mov    %r8d,%ecx
  468925:	c1 e1 07             	shl    $0x7,%ecx
  468928:	48 63 c9             	movslq %ecx,%rcx
  46892b:	62 f1 7c 48 10 44 8b 	vmovups 0x1c0(%rbx,%rcx,4),%zmm0
  468932:	07 
  468933:	62 f1 7c 48 10 4c 8b 	vmovups 0x180(%rbx,%rcx,4),%zmm1
  46893a:	06 
  46893b:	62 f1 7c 48 10 54 8b 	vmovups 0x140(%rbx,%rcx,4),%zmm2
  468942:	05 
  468943:	62 f1 7c 48 10 5c 8b 	vmovups 0x100(%rbx,%rcx,4),%zmm3
  46894a:	04 
  46894b:	62 71 7c 48 10 04 8b 	vmovups (%rbx,%rcx,4),%zmm8
  468952:	62 f1 7c 48 10 7c 8b 	vmovups 0x40(%rbx,%rcx,4),%zmm7
  468959:	01 
  46895a:	62 f1 7c 48 10 74 8b 	vmovups 0x80(%rbx,%rcx,4),%zmm6
  468961:	02 
  468962:	62 f1 7c 48 10 64 8b 	vmovups 0xc0(%rbx,%rcx,4),%zmm4
  468969:	03 
  46896a:	44 89 44 24 e4       	mov    %r8d,-0x1c(%rsp)
  46896f:	44 39 fa             	cmp    %r15d,%edx
  468972:	0f 86 1c fe ff ff    	jbe    468794 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x124>
  468978:	48 8b 4c 24 d8       	mov    -0x28(%rsp),%rcx
  46897d:	4c 8b 44 24 f0       	mov    -0x10(%rsp),%r8
  468982:	42 8b 0c 81          	mov    (%rcx,%r8,4),%ecx
  468986:	45 89 fa             	mov    %r15d,%r10d
  468989:	49 01 ca             	add    %rcx,%r10
  46898c:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  468990:	46 8b 24 97          	mov    (%rdi,%r10,4),%r12d
  468994:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  468999:	45 31 d2             	xor    %r10d,%r10d
  46899c:	45 31 db             	xor    %r11d,%r11d
  46899f:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  4689a6:	00 
  4689a7:	31 c9                	xor    %ecx,%ecx
  4689a9:	31 ed                	xor    %ebp,%ebp
  4689ab:	4d 63 c1             	movslq %r9d,%r8
  4689ae:	4c 89 44 24 f8       	mov    %r8,-0x8(%rsp)
  4689b3:	45 89 fd             	mov    %r15d,%r13d
  4689b6:	89 4c 24 e8          	mov    %ecx,-0x18(%rsp)
  4689ba:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
  4689c0:	a8 01                	test   $0x1,%al
  4689c2:	74 2c                	je     4689f0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x380>
  4689c4:	45 39 e3             	cmp    %r12d,%r11d
  4689c7:	b8 03 00 00 00       	mov    $0x3,%eax
  4689cc:	83 d8 00             	sbb    $0x0,%eax
  4689cf:	41 f6 c2 01          	test   $0x1,%r10b
  4689d3:	41 b8 01 00 00 00    	mov    $0x1,%r8d
  4689d9:	41 0f 44 c0          	cmove  %r8d,%eax
  4689dd:	45 39 dc             	cmp    %r11d,%r12d
  4689e0:	41 0f 42 c0          	cmovb  %r8d,%eax
  4689e4:	45 8d 57 01          	lea    0x1(%r15),%r10d
  4689e8:	a8 01                	test   $0x1,%al
  4689ea:	75 15                	jne    468a01 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x391>
  4689ec:	eb 4f                	jmp    468a3d <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3cd>
  4689ee:	66 90                	xchg   %ax,%ax
  4689f0:	41 0f b6 c2          	movzbl %r10b,%eax
  4689f4:	83 e0 01             	and    $0x1,%eax
  4689f7:	01 c0                	add    %eax,%eax
  4689f9:	45 8d 57 01          	lea    0x1(%r15),%r10d
  4689fd:	a8 01                	test   $0x1,%al
  4689ff:	74 3c                	je     468a3d <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3cd>
  468a01:	44 3b 54 24 ec       	cmp    -0x14(%rsp),%r10d
  468a06:	73 35                	jae    468a3d <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3cd>
  468a08:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  468a0d:	48 8b 5c 24 f0       	mov    -0x10(%rsp),%rbx
  468a12:	45 8b 34 98          	mov    (%r8,%rbx,4),%r14d
  468a16:	44 89 fb             	mov    %r15d,%ebx
  468a19:	4c 01 f3             	add    %r14,%rbx
  468a1c:	8b 5c 9f 04          	mov    0x4(%rdi,%rbx,4),%ebx
  468a20:	48 c1 e3 08          	shl    $0x8,%rbx
  468a24:	0f 18 0c 1e          	prefetcht0 (%rsi,%rbx,1)
  468a28:	0f 18 4c 1e 40       	prefetcht0 0x40(%rsi,%rbx,1)
  468a2d:	0f 18 8c 1e 80 00 00 	prefetcht0 0x80(%rsi,%rbx,1)
  468a34:	00 
  468a35:	0f 18 8c 1e c0 00 00 	prefetcht0 0xc0(%rsi,%rbx,1)
  468a3c:	00 
  468a3d:	45 8d 75 01          	lea    0x1(%r13),%r14d
  468a41:	a8 02                	test   $0x2,%al
  468a43:	74 42                	je     468a87 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x417>
  468a45:	41 39 ee             	cmp    %ebp,%r14d
  468a48:	73 3d                	jae    468a87 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x417>
  468a4a:	89 e9                	mov    %ebp,%ecx
  468a4c:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  468a51:	48 8b 5c 24 f8       	mov    -0x8(%rsp),%rbx
  468a56:	41 8b 2c 98          	mov    (%r8,%rbx,4),%ebp
  468a5a:	44 89 eb             	mov    %r13d,%ebx
  468a5d:	48 01 eb             	add    %rbp,%rbx
  468a60:	89 cd                	mov    %ecx,%ebp
  468a62:	8b 4c 24 e8          	mov    -0x18(%rsp),%ecx
  468a66:	8b 5c 9f 04          	mov    0x4(%rdi,%rbx,4),%ebx
  468a6a:	48 c1 e3 08          	shl    $0x8,%rbx
  468a6e:	0f 18 0c 1e          	prefetcht0 (%rsi,%rbx,1)
  468a72:	0f 18 4c 1e 40       	prefetcht0 0x40(%rsi,%rbx,1)
  468a77:	0f 18 8c 1e 80 00 00 	prefetcht0 0x80(%rsi,%rbx,1)
  468a7e:	00 
  468a7f:	0f 18 8c 1e c0 00 00 	prefetcht0 0xc0(%rsi,%rbx,1)
  468a86:	00 
  468a87:	83 f8 01             	cmp    $0x1,%eax
  468a8a:	0f 84 f0 00 00 00    	je     468b80 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x510>
  468a90:	83 f8 03             	cmp    $0x3,%eax
  468a93:	0f 85 a7 01 00 00    	jne    468c40 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5d0>
  468a99:	45 89 e0             	mov    %r12d,%r8d
  468a9c:	49 c1 e0 08          	shl    $0x8,%r8
  468aa0:	62 a2 7d 48 33 44 06 	vpmovzxwd 0xa0(%rsi,%r8,1),%zmm16
  468aa7:	05 
  468aa8:	62 a2 7d 48 33 4c 06 	vpmovzxwd 0x80(%rsi,%r8,1),%zmm17
  468aaf:	04 
  468ab0:	62 a2 7d 48 33 54 06 	vpmovzxwd 0x60(%rsi,%r8,1),%zmm18
  468ab7:	03 
  468ab8:	62 a2 7d 48 33 5c 06 	vpmovzxwd 0x40(%rsi,%r8,1),%zmm19
  468abf:	02 
  468ac0:	62 a2 7d 48 33 64 06 	vpmovzxwd 0x20(%rsi,%r8,1),%zmm20
  468ac7:	01 
  468ac8:	62 a2 7d 48 33 2c 06 	vpmovzxwd (%rsi,%r8,1),%zmm21
  468acf:	62 a2 7d 48 33 74 06 	vpmovzxwd 0xc0(%rsi,%r8,1),%zmm22
  468ad6:	06 
  468ad7:	62 a2 7d 48 33 7c 06 	vpmovzxwd 0xe0(%rsi,%r8,1),%zmm23
  468ade:	07 
  468adf:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  468ae6:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  468aed:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  468af4:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  468afb:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  468b02:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  468b09:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  468b10:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  468b17:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  468b1d:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  468b23:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  468b29:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  468b2f:	62 b1 44 48 58 fc    	vaddps %zmm20,%zmm7,%zmm7
  468b35:	62 31 3c 48 58 c5    	vaddps %zmm21,%zmm8,%zmm8
  468b3b:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  468b41:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  468b47:	62 31 2c 48 58 d0    	vaddps %zmm16,%zmm10,%zmm10
  468b4d:	62 31 24 48 58 d9    	vaddps %zmm17,%zmm11,%zmm11
  468b53:	62 31 1c 48 58 e2    	vaddps %zmm18,%zmm12,%zmm12
  468b59:	62 31 0c 48 58 f3    	vaddps %zmm19,%zmm14,%zmm14
  468b5f:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  468b65:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  468b6b:	62 31 34 48 58 ce    	vaddps %zmm22,%zmm9,%zmm9
  468b71:	62 b1 54 48 58 ef    	vaddps %zmm23,%zmm5,%zmm5
  468b77:	e9 84 01 00 00       	jmpq   468d00 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x690>
  468b7c:	0f 1f 40 00          	nopl   0x0(%rax)
  468b80:	45 89 e0             	mov    %r12d,%r8d
  468b83:	49 c1 e0 08          	shl    $0x8,%r8
  468b87:	62 a2 7d 48 33 44 06 	vpmovzxwd 0xa0(%rsi,%r8,1),%zmm16
  468b8e:	05 
  468b8f:	62 a2 7d 48 33 4c 06 	vpmovzxwd 0x80(%rsi,%r8,1),%zmm17
  468b96:	04 
  468b97:	62 a2 7d 48 33 54 06 	vpmovzxwd 0x60(%rsi,%r8,1),%zmm18
  468b9e:	03 
  468b9f:	62 a2 7d 48 33 5c 06 	vpmovzxwd 0x40(%rsi,%r8,1),%zmm19
  468ba6:	02 
  468ba7:	62 a2 7d 48 33 64 06 	vpmovzxwd 0x20(%rsi,%r8,1),%zmm20
  468bae:	01 
  468baf:	62 a2 7d 48 33 2c 06 	vpmovzxwd (%rsi,%r8,1),%zmm21
  468bb6:	62 a2 7d 48 33 74 06 	vpmovzxwd 0xc0(%rsi,%r8,1),%zmm22
  468bbd:	06 
  468bbe:	62 a2 7d 48 33 7c 06 	vpmovzxwd 0xe0(%rsi,%r8,1),%zmm23
  468bc5:	07 
  468bc6:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  468bcd:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  468bd4:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  468bdb:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  468be2:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  468be9:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  468bf0:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  468bf7:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  468bfe:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  468c04:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  468c0a:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  468c10:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  468c16:	62 b1 44 48 58 fc    	vaddps %zmm20,%zmm7,%zmm7
  468c1c:	62 31 3c 48 58 c5    	vaddps %zmm21,%zmm8,%zmm8
  468c22:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  468c28:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  468c2e:	e9 cd 00 00 00       	jmpq   468d00 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x690>
  468c33:	66 66 66 66 2e 0f 1f 	data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  468c3a:	84 00 00 00 00 00 
  468c40:	44 89 db             	mov    %r11d,%ebx
  468c43:	48 c1 e3 08          	shl    $0x8,%rbx
  468c47:	62 e2 7d 48 33 44 1e 	vpmovzxwd 0xa0(%rsi,%rbx,1),%zmm16
  468c4e:	05 
  468c4f:	62 e2 7d 48 33 4c 1e 	vpmovzxwd 0x80(%rsi,%rbx,1),%zmm17
  468c56:	04 
  468c57:	62 e2 7d 48 33 54 1e 	vpmovzxwd 0x60(%rsi,%rbx,1),%zmm18
  468c5e:	03 
  468c5f:	62 e2 7d 48 33 5c 1e 	vpmovzxwd 0x40(%rsi,%rbx,1),%zmm19
  468c66:	02 
  468c67:	62 e2 7d 48 33 64 1e 	vpmovzxwd 0x20(%rsi,%rbx,1),%zmm20
  468c6e:	01 
  468c6f:	62 e2 7d 48 33 2c 1e 	vpmovzxwd (%rsi,%rbx,1),%zmm21
  468c76:	62 e2 7d 48 33 74 1e 	vpmovzxwd 0xc0(%rsi,%rbx,1),%zmm22
  468c7d:	06 
  468c7e:	62 e2 7d 48 33 7c 1e 	vpmovzxwd 0xe0(%rsi,%rbx,1),%zmm23
  468c85:	07 
  468c86:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  468c8d:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  468c94:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  468c9b:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  468ca2:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  468ca9:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  468cb0:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  468cb7:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  468cbe:	62 31 2c 48 58 d0    	vaddps %zmm16,%zmm10,%zmm10
  468cc4:	62 31 24 48 58 d9    	vaddps %zmm17,%zmm11,%zmm11
  468cca:	62 31 1c 48 58 e2    	vaddps %zmm18,%zmm12,%zmm12
  468cd0:	62 31 0c 48 58 f3    	vaddps %zmm19,%zmm14,%zmm14
  468cd6:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  468cdc:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  468ce2:	62 31 34 48 58 ce    	vaddps %zmm22,%zmm9,%zmm9
  468ce8:	62 b1 54 48 58 ef    	vaddps %zmm23,%zmm5,%zmm5
  468cee:	a8 01                	test   $0x1,%al
  468cf0:	74 2e                	je     468d20 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6b0>
  468cf2:	66 66 66 66 66 2e 0f 	data16 data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  468cf9:	1f 84 00 00 00 00 00 
  468d00:	41 39 d2             	cmp    %edx,%r10d
  468d03:	73 18                	jae    468d1d <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6ad>
  468d05:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  468d0a:	48 8b 5c 24 f0       	mov    -0x10(%rsp),%rbx
  468d0f:	45 8b 04 98          	mov    (%r8,%rbx,4),%r8d
  468d13:	45 89 d4             	mov    %r10d,%r12d
  468d16:	4d 01 c4             	add    %r8,%r12
  468d19:	46 8b 24 a7          	mov    (%rdi,%r12,4),%r12d
  468d1d:	45 89 d7             	mov    %r10d,%r15d
  468d20:	a8 02                	test   $0x2,%al
  468d22:	74 20                	je     468d44 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6d4>
  468d24:	41 39 ce             	cmp    %ecx,%r14d
  468d27:	73 18                	jae    468d41 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6d1>
  468d29:	48 8b 44 24 d8       	mov    -0x28(%rsp),%rax
  468d2e:	4c 8b 44 24 f8       	mov    -0x8(%rsp),%r8
  468d33:	42 8b 04 80          	mov    (%rax,%r8,4),%eax
  468d37:	45 89 f2             	mov    %r14d,%r10d
  468d3a:	49 01 c2             	add    %rax,%r10
  468d3d:	46 8b 1c 97          	mov    (%rdi,%r10,4),%r11d
  468d41:	45 89 f5             	mov    %r14d,%r13d
  468d44:	45 85 c9             	test   %r9d,%r9d
  468d47:	0f 99 c0             	setns  %al
  468d4a:	41 39 cd             	cmp    %ecx,%r13d
  468d4d:	41 0f 92 c2          	setb   %r10b
  468d51:	41 20 c2             	and    %al,%r10b
  468d54:	41 39 d7             	cmp    %edx,%r15d
  468d57:	0f 92 c0             	setb   %al
  468d5a:	0f 82 60 fc ff ff    	jb     4689c0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x350>
  468d60:	45 84 d2             	test   %r10b,%r10b
  468d63:	0f 85 57 fc ff ff    	jne    4689c0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x350>
  468d69:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  468d6e:	48 8b 54 24 70       	mov    0x70(%rsp),%rdx
  468d73:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  468d78:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
  468d7d:	80 7c 24 d3 00       	cmpb   $0x0,-0x2d(%rsp)
  468d82:	8b 4c 24 e4          	mov    -0x1c(%rsp),%ecx
  468d86:	8b 7c 24 40          	mov    0x40(%rsp),%edi
  468d8a:	0f 84 b8 01 00 00    	je     468f48 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8d8>
  468d90:	80 7a 2a 00          	cmpb   $0x0,0x2a(%rdx)
  468d94:	0f 84 7d 03 00 00    	je     469117 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xaa7>
  468d9a:	39 7c 24 ec          	cmp    %edi,-0x14(%rsp)
  468d9e:	0f 87 73 03 00 00    	ja     469117 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xaa7>
  468da4:	62 d1 7d 40 72 d0 10 	vpsrld $0x10,%zmm8,%zmm16
  468dab:	c1 e1 07             	shl    $0x7,%ecx
  468dae:	48 63 f9             	movslq %ecx,%rdi
  468db1:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468db5:	62 e2 7e 48 33 04 7e 	vpmovdw %zmm16,(%rsi,%rdi,2)
  468dbc:	74 1e                	je     468ddc <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x76c>
  468dbe:	62 e1 3c 58 54 05 30 	vandps 0x3cd30(%rip){1to16},%zmm8,%zmm16        # 4a5af8 <__dso_handle+0x30>
  468dc5:	cd 03 00 
  468dc8:	62 31 3c 48 5c c0    	vsubps %zmm16,%zmm8,%zmm8
  468dce:	62 d1 3d 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm8
  468dd5:	62 72 7e 48 33 04 78 	vpmovdw %zmm8,(%rax,%rdi,2)
  468ddc:	62 f1 3d 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm8
  468de3:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468de7:	62 72 7e 48 33 44 7e 	vpmovdw %zmm8,0x20(%rsi,%rdi,2)
  468dee:	01 
  468def:	74 1f                	je     468e10 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x7a0>
  468df1:	62 71 44 58 54 05 fd 	vandps 0x3ccfd(%rip){1to16},%zmm7,%zmm8        # 4a5af8 <__dso_handle+0x30>
  468df8:	cc 03 00 
  468dfb:	62 d1 44 48 5c f8    	vsubps %zmm8,%zmm7,%zmm7
  468e01:	62 f1 45 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm7
  468e08:	62 f2 7e 48 33 7c 78 	vpmovdw %zmm7,0x20(%rax,%rdi,2)
  468e0f:	01 
  468e10:	62 f1 45 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm7
  468e17:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468e1b:	62 f2 7e 48 33 7c 7e 	vpmovdw %zmm7,0x40(%rsi,%rdi,2)
  468e22:	02 
  468e23:	74 1f                	je     468e44 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x7d4>
  468e25:	62 f1 4c 58 54 3d c9 	vandps 0x3ccc9(%rip){1to16},%zmm6,%zmm7        # 4a5af8 <__dso_handle+0x30>
  468e2c:	cc 03 00 
  468e2f:	62 f1 4c 48 5c f7    	vsubps %zmm7,%zmm6,%zmm6
  468e35:	62 f1 4d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm6
  468e3c:	62 f2 7e 48 33 74 78 	vpmovdw %zmm6,0x40(%rax,%rdi,2)
  468e43:	02 
  468e44:	62 f1 4d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm6
  468e4b:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468e4f:	62 f2 7e 48 33 74 7e 	vpmovdw %zmm6,0x60(%rsi,%rdi,2)
  468e56:	03 
  468e57:	74 1f                	je     468e78 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x808>
  468e59:	62 f1 5c 58 54 35 95 	vandps 0x3cc95(%rip){1to16},%zmm4,%zmm6        # 4a5af8 <__dso_handle+0x30>
  468e60:	cc 03 00 
  468e63:	62 f1 5c 48 5c e6    	vsubps %zmm6,%zmm4,%zmm4
  468e69:	62 f1 5d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm4
  468e70:	62 f2 7e 48 33 64 78 	vpmovdw %zmm4,0x60(%rax,%rdi,2)
  468e77:	03 
  468e78:	62 f1 5d 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm4
  468e7f:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468e83:	62 f2 7e 48 33 64 7e 	vpmovdw %zmm4,0x80(%rsi,%rdi,2)
  468e8a:	04 
  468e8b:	74 1f                	je     468eac <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x83c>
  468e8d:	62 f1 64 58 54 25 61 	vandps 0x3cc61(%rip){1to16},%zmm3,%zmm4        # 4a5af8 <__dso_handle+0x30>
  468e94:	cc 03 00 
  468e97:	62 f1 64 48 5c dc    	vsubps %zmm4,%zmm3,%zmm3
  468e9d:	62 f1 65 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm3
  468ea4:	62 f2 7e 48 33 5c 78 	vpmovdw %zmm3,0x80(%rax,%rdi,2)
  468eab:	04 
  468eac:	62 f1 65 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm3
  468eb3:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468eb7:	62 f2 7e 48 33 5c 7e 	vpmovdw %zmm3,0xa0(%rsi,%rdi,2)
  468ebe:	05 
  468ebf:	74 1f                	je     468ee0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x870>
  468ec1:	62 f1 6c 58 54 1d 2d 	vandps 0x3cc2d(%rip){1to16},%zmm2,%zmm3        # 4a5af8 <__dso_handle+0x30>
  468ec8:	cc 03 00 
  468ecb:	62 f1 6c 48 5c d3    	vsubps %zmm3,%zmm2,%zmm2
  468ed1:	62 f1 6d 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm2
  468ed8:	62 f2 7e 48 33 54 78 	vpmovdw %zmm2,0xa0(%rax,%rdi,2)
  468edf:	05 
  468ee0:	62 f1 6d 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm2
  468ee7:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468eeb:	62 f2 7e 48 33 54 7e 	vpmovdw %zmm2,0xc0(%rsi,%rdi,2)
  468ef2:	06 
  468ef3:	74 1f                	je     468f14 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8a4>
  468ef5:	62 f1 74 58 54 15 f9 	vandps 0x3cbf9(%rip){1to16},%zmm1,%zmm2        # 4a5af8 <__dso_handle+0x30>
  468efc:	cb 03 00 
  468eff:	62 f1 74 48 5c ca    	vsubps %zmm2,%zmm1,%zmm1
  468f05:	62 f1 75 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm1
  468f0c:	62 f2 7e 48 33 4c 78 	vpmovdw %zmm1,0xc0(%rax,%rdi,2)
  468f13:	06 
  468f14:	62 f1 75 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm1
  468f1b:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468f1f:	62 f2 7e 48 33 4c 7e 	vpmovdw %zmm1,0xe0(%rsi,%rdi,2)
  468f26:	07 
  468f27:	74 1f                	je     468f48 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8d8>
  468f29:	62 f1 7c 58 54 0d c5 	vandps 0x3cbc5(%rip){1to16},%zmm0,%zmm1        # 4a5af8 <__dso_handle+0x30>
  468f30:	cb 03 00 
  468f33:	62 f1 7c 48 5c c1    	vsubps %zmm1,%zmm0,%zmm0
  468f39:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  468f40:	62 f2 7e 48 33 44 78 	vpmovdw %zmm0,0xe0(%rax,%rdi,2)
  468f47:	07 
  468f48:	80 7c 24 d4 00       	cmpb   $0x0,-0x2c(%rsp)
  468f4d:	8b 4c 24 e8          	mov    -0x18(%rsp),%ecx
  468f51:	0f 84 5c 02 00 00    	je     4691b3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  468f57:	80 7a 2a 00          	cmpb   $0x0,0x2a(%rdx)
  468f5b:	0f 84 0c 02 00 00    	je     46916d <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xafd>
  468f61:	39 e9                	cmp    %ebp,%ecx
  468f63:	0f 85 04 02 00 00    	jne    46916d <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xafd>
  468f69:	62 d1 7d 48 72 d5 10 	vpsrld $0x10,%zmm13,%zmm0
  468f70:	41 c1 e1 07          	shl    $0x7,%r9d
  468f74:	49 63 c9             	movslq %r9d,%rcx
  468f77:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468f7b:	62 f2 7e 48 33 04 4e 	vpmovdw %zmm0,(%rsi,%rcx,2)
  468f82:	74 1e                	je     468fa2 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x932>
  468f84:	62 f1 14 58 54 05 6a 	vandps 0x3cb6a(%rip){1to16},%zmm13,%zmm0        # 4a5af8 <__dso_handle+0x30>
  468f8b:	cb 03 00 
  468f8e:	62 f1 14 48 5c c0    	vsubps %zmm0,%zmm13,%zmm0
  468f94:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  468f9b:	62 f2 7e 48 33 04 48 	vpmovdw %zmm0,(%rax,%rcx,2)
  468fa2:	62 d1 7d 48 72 d7 10 	vpsrld $0x10,%zmm15,%zmm0
  468fa9:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468fad:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x20(%rsi,%rcx,2)
  468fb4:	01 
  468fb5:	74 1f                	je     468fd6 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x966>
  468fb7:	62 f1 04 58 54 05 37 	vandps 0x3cb37(%rip){1to16},%zmm15,%zmm0        # 4a5af8 <__dso_handle+0x30>
  468fbe:	cb 03 00 
  468fc1:	62 f1 04 48 5c c0    	vsubps %zmm0,%zmm15,%zmm0
  468fc7:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  468fce:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x20(%rax,%rcx,2)
  468fd5:	01 
  468fd6:	62 d1 7d 48 72 d6 10 	vpsrld $0x10,%zmm14,%zmm0
  468fdd:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  468fe1:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x40(%rsi,%rcx,2)
  468fe8:	02 
  468fe9:	74 1f                	je     46900a <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x99a>
  468feb:	62 f1 0c 58 54 05 03 	vandps 0x3cb03(%rip){1to16},%zmm14,%zmm0        # 4a5af8 <__dso_handle+0x30>
  468ff2:	cb 03 00 
  468ff5:	62 f1 0c 48 5c c0    	vsubps %zmm0,%zmm14,%zmm0
  468ffb:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  469002:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x40(%rax,%rcx,2)
  469009:	02 
  46900a:	62 d1 7d 48 72 d4 10 	vpsrld $0x10,%zmm12,%zmm0
  469011:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  469015:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x60(%rsi,%rcx,2)
  46901c:	03 
  46901d:	74 1f                	je     46903e <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x9ce>
  46901f:	62 f1 1c 58 54 05 cf 	vandps 0x3cacf(%rip){1to16},%zmm12,%zmm0        # 4a5af8 <__dso_handle+0x30>
  469026:	ca 03 00 
  469029:	62 f1 1c 48 5c c0    	vsubps %zmm0,%zmm12,%zmm0
  46902f:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  469036:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x60(%rax,%rcx,2)
  46903d:	03 
  46903e:	62 d1 7d 48 72 d3 10 	vpsrld $0x10,%zmm11,%zmm0
  469045:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  469049:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x80(%rsi,%rcx,2)
  469050:	04 
  469051:	74 1f                	je     469072 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa02>
  469053:	62 f1 24 58 54 05 9b 	vandps 0x3ca9b(%rip){1to16},%zmm11,%zmm0        # 4a5af8 <__dso_handle+0x30>
  46905a:	ca 03 00 
  46905d:	62 f1 24 48 5c c0    	vsubps %zmm0,%zmm11,%zmm0
  469063:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46906a:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x80(%rax,%rcx,2)
  469071:	04 
  469072:	62 d1 7d 48 72 d2 10 	vpsrld $0x10,%zmm10,%zmm0
  469079:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46907d:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xa0(%rsi,%rcx,2)
  469084:	05 
  469085:	74 1f                	je     4690a6 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa36>
  469087:	62 f1 2c 58 54 05 67 	vandps 0x3ca67(%rip){1to16},%zmm10,%zmm0        # 4a5af8 <__dso_handle+0x30>
  46908e:	ca 03 00 
  469091:	62 f1 2c 48 5c c0    	vsubps %zmm0,%zmm10,%zmm0
  469097:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46909e:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xa0(%rax,%rcx,2)
  4690a5:	05 
  4690a6:	62 d1 7d 48 72 d1 10 	vpsrld $0x10,%zmm9,%zmm0
  4690ad:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  4690b1:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xc0(%rsi,%rcx,2)
  4690b8:	06 
  4690b9:	74 1f                	je     4690da <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa6a>
  4690bb:	62 f1 34 58 54 05 33 	vandps 0x3ca33(%rip){1to16},%zmm9,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4690c2:	ca 03 00 
  4690c5:	62 f1 34 48 5c c0    	vsubps %zmm0,%zmm9,%zmm0
  4690cb:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4690d2:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xc0(%rax,%rcx,2)
  4690d9:	06 
  4690da:	62 f1 7d 48 72 d5 10 	vpsrld $0x10,%zmm5,%zmm0
  4690e1:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  4690e5:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xe0(%rsi,%rcx,2)
  4690ec:	07 
  4690ed:	0f 84 c0 00 00 00    	je     4691b3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  4690f3:	62 f1 54 58 54 05 fb 	vandps 0x3c9fb(%rip){1to16},%zmm5,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4690fa:	c9 03 00 
  4690fd:	62 f1 54 48 5c c0    	vsubps %zmm0,%zmm5,%zmm0
  469103:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46910a:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xe0(%rax,%rcx,2)
  469111:	07 
  469112:	e9 9c 00 00 00       	jmpq   4691b3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  469117:	c1 e1 07             	shl    $0x7,%ecx
  46911a:	48 63 f9             	movslq %ecx,%rdi
  46911d:	62 f1 7c 48 11 74 bb 	vmovups %zmm6,0x80(%rbx,%rdi,4)
  469124:	02 
  469125:	62 f1 7c 48 11 7c bb 	vmovups %zmm7,0x40(%rbx,%rdi,4)
  46912c:	01 
  46912d:	62 71 7c 48 11 04 bb 	vmovups %zmm8,(%rbx,%rdi,4)
  469134:	62 f1 7c 48 11 64 bb 	vmovups %zmm4,0xc0(%rbx,%rdi,4)
  46913b:	03 
  46913c:	62 f1 7c 48 11 5c bb 	vmovups %zmm3,0x100(%rbx,%rdi,4)
  469143:	04 
  469144:	62 f1 7c 48 11 54 bb 	vmovups %zmm2,0x140(%rbx,%rdi,4)
  46914b:	05 
  46914c:	62 f1 7c 48 11 4c bb 	vmovups %zmm1,0x180(%rbx,%rdi,4)
  469153:	06 
  469154:	62 f1 7c 48 11 44 bb 	vmovups %zmm0,0x1c0(%rbx,%rdi,4)
  46915b:	07 
  46915c:	80 7c 24 d4 00       	cmpb   $0x0,-0x2c(%rsp)
  469161:	8b 4c 24 e8          	mov    -0x18(%rsp),%ecx
  469165:	0f 85 ec fd ff ff    	jne    468f57 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8e7>
  46916b:	eb 46                	jmp    4691b3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  46916d:	41 c1 e1 07          	shl    $0x7,%r9d
  469171:	49 63 c1             	movslq %r9d,%rax
  469174:	62 71 7c 48 11 74 83 	vmovups %zmm14,0x80(%rbx,%rax,4)
  46917b:	02 
  46917c:	62 71 7c 48 11 7c 83 	vmovups %zmm15,0x40(%rbx,%rax,4)
  469183:	01 
  469184:	62 71 7c 48 11 2c 83 	vmovups %zmm13,(%rbx,%rax,4)
  46918b:	62 71 7c 48 11 64 83 	vmovups %zmm12,0xc0(%rbx,%rax,4)
  469192:	03 
  469193:	62 71 7c 48 11 5c 83 	vmovups %zmm11,0x100(%rbx,%rax,4)
  46919a:	04 
  46919b:	62 71 7c 48 11 54 83 	vmovups %zmm10,0x140(%rbx,%rax,4)
  4691a2:	05 
  4691a3:	62 71 7c 48 11 4c 83 	vmovups %zmm9,0x180(%rbx,%rax,4)
  4691aa:	06 
  4691ab:	62 f1 7c 48 11 6c 83 	vmovups %zmm5,0x1c0(%rbx,%rax,4)
  4691b2:	07 
  4691b3:	5b                   	pop    %rbx
  4691b4:	41 5c                	pop    %r12
  4691b6:	41 5d                	pop    %r13
  4691b8:	41 5e                	pop    %r14
  4691ba:	41 5f                	pop    %r15
  4691bc:	5d                   	pop    %rbp
  4691bd:	c5 f8 77             	vzeroupper 
  4691c0:	c3                   	retq   
  4691c1:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
  4691c8:	00 00 00 
  4691cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

