0000000000464790 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)>:
  464790:	55                   	push   %rbp
  464791:	41 57                	push   %r15
  464793:	41 56                	push   %r14
  464795:	41 55                	push   %r13
  464797:	41 54                	push   %r12
  464799:	53                   	push   %rbx
  46479a:	48 89 54 24 c8       	mov    %rdx,-0x38(%rsp)
  46479f:	4c 8b 5c 24 58       	mov    0x58(%rsp),%r11
  4647a4:	8b 5c 24 48          	mov    0x48(%rsp),%ebx
  4647a8:	44 8b 7c 24 40       	mov    0x40(%rsp),%r15d
  4647ad:	8b 6c 24 38          	mov    0x38(%rsp),%ebp
  4647b1:	48 8b 44 24 78       	mov    0x78(%rsp),%rax
  4647b6:	4c 8b 30             	mov    (%rax),%r14
  4647b9:	4d 8d 56 01          	lea    0x1(%r14),%r10
  4647bd:	4c 89 54 24 e0       	mov    %r10,-0x20(%rsp)
  4647c2:	4c 89 10             	mov    %r10,(%rax)
  4647c5:	45 0f b6 26          	movzbl (%r14),%r12d
  4647c9:	45 8d 54 24 03       	lea    0x3(%r12),%r10d
  4647ce:	41 c1 ea 02          	shr    $0x2,%r10d
  4647d2:	4f 8d 54 16 01       	lea    0x1(%r14,%r10,1),%r10
  4647d7:	4c 89 10             	mov    %r10,(%rax)
  4647da:	49 63 c0             	movslq %r8d,%rax
  4647dd:	48 89 44 24 f8       	mov    %rax,-0x8(%rsp)
  4647e2:	44 8b 2c 81          	mov    (%rcx,%rax,4),%r13d
  4647e6:	41 39 dd             	cmp    %ebx,%r13d
  4647e9:	44 0f 43 eb          	cmovae %ebx,%r13d
  4647ed:	45 39 fd             	cmp    %r15d,%r13d
  4647f0:	44 89 f8             	mov    %r15d,%eax
  4647f3:	41 0f 42 c5          	cmovb  %r13d,%eax
  4647f7:	39 e8                	cmp    %ebp,%eax
  4647f9:	0f b6 44 24 50       	movzbl 0x50(%rsp),%eax
  4647fe:	41 0f 97 c6          	seta   %r14b
  464802:	41 08 c6             	or     %al,%r14b
  464805:	45 85 c9             	test   %r9d,%r9d
  464808:	0f 88 90 00 00 00    	js     46489e <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x10e>
  46480e:	45 89 ca             	mov    %r9d,%r10d
  464811:	42 8b 0c 91          	mov    (%rcx,%r10,4),%ecx
  464815:	39 d9                	cmp    %ebx,%ecx
  464817:	0f 42 d9             	cmovb  %ecx,%ebx
  46481a:	44 39 fb             	cmp    %r15d,%ebx
  46481d:	44 89 f9             	mov    %r15d,%ecx
  464820:	0f 42 cb             	cmovb  %ebx,%ecx
  464823:	89 4c 24 c0          	mov    %ecx,-0x40(%rsp)
  464827:	39 e9                	cmp    %ebp,%ecx
  464829:	0f 97 c1             	seta   %cl
  46482c:	08 c1                	or     %al,%cl
  46482e:	45 84 f6             	test   %r14b,%r14b
  464831:	0f 84 a6 00 00 00    	je     4648dd <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x14d>
  464837:	84 c0                	test   %al,%al
  464839:	0f 84 fc 00 00 00    	je     46493b <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x1ab>
  46483f:	c4 41 10 57 ed       	vxorps %xmm13,%xmm13,%xmm13
  464844:	c4 41 00 57 ff       	vxorps %xmm15,%xmm15,%xmm15
  464849:	c4 41 18 57 e4       	vxorps %xmm12,%xmm12,%xmm12
  46484e:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  464853:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  464857:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  46485b:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  46485f:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  464863:	84 c9                	test   %cl,%cl
  464865:	0f 84 21 01 00 00    	je     46498c <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x1fc>
  46486b:	b1 01                	mov    $0x1,%cl
  46486d:	89 4c 24 bc          	mov    %ecx,-0x44(%rsp)
  464871:	84 c0                	test   %al,%al
  464873:	74 7a                	je     4648ef <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x15f>
  464875:	c4 41 28 57 d2       	vxorps %xmm10,%xmm10,%xmm10
  46487a:	c4 41 08 57 f6       	vxorps %xmm14,%xmm14,%xmm14
  46487f:	c4 41 20 57 db       	vxorps %xmm11,%xmm11,%xmm11
  464884:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  464889:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  46488d:	c5 d0 57 ed          	vxorps %xmm5,%xmm5,%xmm5
  464891:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  464895:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  464899:	e9 54 01 00 00       	jmpq   4649f2 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x262>
  46489e:	45 84 f6             	test   %r14b,%r14b
  4648a1:	0f 84 2c 09 00 00    	je     4651d3 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa43>
  4648a7:	41 b6 01             	mov    $0x1,%r14b
  4648aa:	84 c0                	test   %al,%al
  4648ac:	0f 84 e7 00 00 00    	je     464999 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x209>
  4648b2:	c4 41 10 57 ed       	vxorps %xmm13,%xmm13,%xmm13
  4648b7:	31 db                	xor    %ebx,%ebx
  4648b9:	c4 41 00 57 ff       	vxorps %xmm15,%xmm15,%xmm15
  4648be:	c4 41 18 57 e4       	vxorps %xmm12,%xmm12,%xmm12
  4648c3:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  4648c8:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  4648cc:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  4648d0:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  4648d4:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  4648d8:	e9 05 01 00 00       	jmpq   4649e2 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x252>
  4648dd:	84 c9                	test   %cl,%cl
  4648df:	0f 84 ee 08 00 00    	je     4651d3 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa43>
  4648e5:	b1 01                	mov    $0x1,%cl
  4648e7:	89 4c 24 bc          	mov    %ecx,-0x44(%rsp)
  4648eb:	84 c0                	test   %al,%al
  4648ed:	75 86                	jne    464875 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xe5>
  4648ef:	44 89 c8             	mov    %r9d,%eax
  4648f2:	c1 e0 07             	shl    $0x7,%eax
  4648f5:	48 98                	cltq   
  4648f7:	62 d1 7c 48 10 4c 83 	vmovups 0x1c0(%r11,%rax,4),%zmm1
  4648fe:	07 
  4648ff:	62 d1 7c 48 10 5c 83 	vmovups 0x180(%r11,%rax,4),%zmm3
  464906:	06 
  464907:	62 d1 7c 48 10 6c 83 	vmovups 0x140(%r11,%rax,4),%zmm5
  46490e:	05 
  46490f:	62 d1 7c 48 10 7c 83 	vmovups 0x100(%r11,%rax,4),%zmm7
  464916:	04 
  464917:	62 51 7c 48 10 14 83 	vmovups (%r11,%rax,4),%zmm10
  46491e:	62 51 7c 48 10 74 83 	vmovups 0x40(%r11,%rax,4),%zmm14
  464925:	01 
  464926:	62 51 7c 48 10 5c 83 	vmovups 0x80(%r11,%rax,4),%zmm11
  46492d:	02 
  46492e:	62 51 7c 48 10 44 83 	vmovups 0xc0(%r11,%rax,4),%zmm8
  464935:	03 
  464936:	e9 b7 00 00 00       	jmpq   4649f2 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x262>
  46493b:	45 89 c2             	mov    %r8d,%r10d
  46493e:	41 c1 e2 07          	shl    $0x7,%r10d
  464942:	4d 63 d2             	movslq %r10d,%r10
  464945:	62 91 7c 48 10 44 93 	vmovups 0x1c0(%r11,%r10,4),%zmm0
  46494c:	07 
  46494d:	62 91 7c 48 10 54 93 	vmovups 0x180(%r11,%r10,4),%zmm2
  464954:	06 
  464955:	62 91 7c 48 10 64 93 	vmovups 0x140(%r11,%r10,4),%zmm4
  46495c:	05 
  46495d:	62 91 7c 48 10 74 93 	vmovups 0x100(%r11,%r10,4),%zmm6
  464964:	04 
  464965:	62 11 7c 48 10 2c 93 	vmovups (%r11,%r10,4),%zmm13
  46496c:	62 11 7c 48 10 7c 93 	vmovups 0x40(%r11,%r10,4),%zmm15
  464973:	01 
  464974:	62 11 7c 48 10 64 93 	vmovups 0x80(%r11,%r10,4),%zmm12
  46497b:	02 
  46497c:	62 11 7c 48 10 4c 93 	vmovups 0xc0(%r11,%r10,4),%zmm9
  464983:	03 
  464984:	84 c9                	test   %cl,%cl
  464986:	0f 85 df fe ff ff    	jne    46486b <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xdb>
  46498c:	41 b6 01             	mov    $0x1,%r14b
  46498f:	c7 44 24 bc 00 00 00 	movl   $0x0,-0x44(%rsp)
  464996:	00 
  464997:	eb 59                	jmp    4649f2 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x262>
  464999:	44 89 c0             	mov    %r8d,%eax
  46499c:	c1 e0 07             	shl    $0x7,%eax
  46499f:	48 98                	cltq   
  4649a1:	62 d1 7c 48 10 44 83 	vmovups 0x1c0(%r11,%rax,4),%zmm0
  4649a8:	07 
  4649a9:	62 d1 7c 48 10 54 83 	vmovups 0x180(%r11,%rax,4),%zmm2
  4649b0:	06 
  4649b1:	62 d1 7c 48 10 64 83 	vmovups 0x140(%r11,%rax,4),%zmm4
  4649b8:	05 
  4649b9:	62 d1 7c 48 10 74 83 	vmovups 0x100(%r11,%rax,4),%zmm6
  4649c0:	04 
  4649c1:	62 51 7c 48 10 2c 83 	vmovups (%r11,%rax,4),%zmm13
  4649c8:	62 51 7c 48 10 7c 83 	vmovups 0x40(%r11,%rax,4),%zmm15
  4649cf:	01 
  4649d0:	62 51 7c 48 10 64 83 	vmovups 0x80(%r11,%rax,4),%zmm12
  4649d7:	02 
  4649d8:	62 51 7c 48 10 4c 83 	vmovups 0xc0(%r11,%rax,4),%zmm9
  4649df:	03 
  4649e0:	31 db                	xor    %ebx,%ebx
  4649e2:	c7 44 24 c0 00 00 00 	movl   $0x0,-0x40(%rsp)
  4649e9:	00 
  4649ea:	c7 44 24 bc 00 00 00 	movl   $0x0,-0x44(%rsp)
  4649f1:	00 
  4649f2:	44 88 74 24 bb       	mov    %r14b,-0x45(%rsp)
  4649f7:	44 89 44 24 c4       	mov    %r8d,-0x3c(%rsp)
  4649fc:	45 85 e4             	test   %r12d,%r12d
  4649ff:	44 89 4c 24 d4       	mov    %r9d,-0x2c(%rsp)
  464a04:	0f 84 82 03 00 00    	je     464d8c <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x5fc>
  464a0a:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  464a0e:	4d 63 d9             	movslq %r9d,%r11
  464a11:	44 89 e0             	mov    %r12d,%eax
  464a14:	48 89 44 24 d8       	mov    %rax,-0x28(%rsp)
  464a19:	31 c0                	xor    %eax,%eax
  464a1b:	31 d2                	xor    %edx,%edx
  464a1d:	45 31 c9             	xor    %r9d,%r9d
  464a20:	41 89 ee             	mov    %ebp,%r14d
  464a23:	eb 28                	jmp    464a4d <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x2bd>
  464a25:	66 66 2e 0f 1f 84 00 	data16 nopw %cs:0x0(%rax,%rax,1)
  464a2c:	00 00 00 00 
  464a30:	48 8b 44 24 e8       	mov    -0x18(%rsp),%rax
  464a35:	48 83 c0 04          	add    $0x4,%rax
  464a39:	4c 8b 64 24 f0       	mov    -0x10(%rsp),%r12
  464a3e:	41 83 c4 fc          	add    $0xfffffffc,%r12d
  464a42:	48 3b 44 24 d8       	cmp    -0x28(%rsp),%rax
  464a47:	0f 83 3f 03 00 00    	jae    464d8c <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x5fc>
  464a4d:	41 83 fc 04          	cmp    $0x4,%r12d
  464a51:	41 bf 04 00 00 00    	mov    $0x4,%r15d
  464a57:	4c 89 64 24 f0       	mov    %r12,-0x10(%rsp)
  464a5c:	45 0f 42 fc          	cmovb  %r12d,%r15d
  464a60:	41 83 ff 01          	cmp    $0x1,%r15d
  464a64:	41 83 d7 00          	adc    $0x0,%r15d
  464a68:	48 89 44 24 e8       	mov    %rax,-0x18(%rsp)
  464a6d:	48 c1 e8 02          	shr    $0x2,%rax
  464a71:	48 8b 4c 24 e0       	mov    -0x20(%rsp),%rcx
  464a76:	44 0f b6 24 01       	movzbl (%rcx,%rax,1),%r12d
  464a7b:	e9 c7 00 00 00       	jmpq   464b47 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3b7>
  464a80:	41 89 d2             	mov    %edx,%r10d
  464a83:	49 c1 e2 08          	shl    $0x8,%r10
  464a87:	62 a2 7d 48 33 44 16 	vpmovzxwd 0xa0(%rsi,%r10,1),%zmm16
  464a8e:	05 
  464a8f:	62 a2 7d 48 33 4c 16 	vpmovzxwd 0x80(%rsi,%r10,1),%zmm17
  464a96:	04 
  464a97:	62 a2 7d 48 33 54 16 	vpmovzxwd 0x60(%rsi,%r10,1),%zmm18
  464a9e:	03 
  464a9f:	62 a2 7d 48 33 5c 16 	vpmovzxwd 0x40(%rsi,%r10,1),%zmm19
  464aa6:	02 
  464aa7:	62 a2 7d 48 33 64 16 	vpmovzxwd 0x20(%rsi,%r10,1),%zmm20
  464aae:	01 
  464aaf:	62 a2 7d 48 33 2c 16 	vpmovzxwd (%rsi,%r10,1),%zmm21
  464ab6:	62 a2 7d 48 33 74 16 	vpmovzxwd 0xc0(%rsi,%r10,1),%zmm22
  464abd:	06 
  464abe:	62 a2 7d 48 33 7c 16 	vpmovzxwd 0xe0(%rsi,%r10,1),%zmm23
  464ac5:	07 
  464ac6:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  464acd:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  464ad4:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  464adb:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  464ae2:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  464ae9:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  464af0:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  464af7:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  464afe:	62 b1 54 48 58 e8    	vaddps %zmm16,%zmm5,%zmm5
  464b04:	62 b1 44 48 58 f9    	vaddps %zmm17,%zmm7,%zmm7
  464b0a:	62 31 3c 48 58 c2    	vaddps %zmm18,%zmm8,%zmm8
  464b10:	62 31 24 48 58 db    	vaddps %zmm19,%zmm11,%zmm11
  464b16:	62 31 0c 48 58 f4    	vaddps %zmm20,%zmm14,%zmm14
  464b1c:	62 31 2c 48 58 d5    	vaddps %zmm21,%zmm10,%zmm10
  464b22:	62 b1 64 48 58 de    	vaddps %zmm22,%zmm3,%zmm3
  464b28:	62 b1 74 48 58 cf    	vaddps %zmm23,%zmm1,%zmm1
  464b2e:	41 c0 ec 02          	shr    $0x2,%r12b
  464b32:	0f b6 c0             	movzbl %al,%eax
  464b35:	01 c5                	add    %eax,%ebp
  464b37:	80 f9 02             	cmp    $0x2,%cl
  464b3a:	41 83 de ff          	sbb    $0xffffffff,%r14d
  464b3e:	41 ff cf             	dec    %r15d
  464b41:	0f 84 e9 fe ff ff    	je     464a30 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x2a0>
  464b47:	44 89 e0             	mov    %r12d,%eax
  464b4a:	24 01                	and    $0x1,%al
  464b4c:	75 22                	jne    464b70 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3e0>
  464b4e:	48 8b 4c 24 c8       	mov    -0x38(%rsp),%rcx
  464b53:	42 8b 0c 99          	mov    (%rcx,%r11,4),%ecx
  464b57:	45 89 f0             	mov    %r14d,%r8d
  464b5a:	49 01 c8             	add    %rcx,%r8
  464b5d:	42 8b 14 87          	mov    (%rdi,%r8,4),%edx
  464b61:	eb 4e                	jmp    464bb1 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x421>
  464b63:	66 66 66 66 2e 0f 1f 	data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  464b6a:	84 00 00 00 00 00 
  464b70:	48 8b 4c 24 f8       	mov    -0x8(%rsp),%rcx
  464b75:	4c 8b 44 24 c8       	mov    -0x38(%rsp),%r8
  464b7a:	45 8b 0c 88          	mov    (%r8,%rcx,4),%r9d
  464b7e:	89 e9                	mov    %ebp,%ecx
  464b80:	4c 01 c9             	add    %r9,%rcx
  464b83:	44 8b 0c 8f          	mov    (%rdi,%rcx,4),%r9d
  464b87:	44 8d 55 01          	lea    0x1(%rbp),%r10d
  464b8b:	45 39 ea             	cmp    %r13d,%r10d
  464b8e:	73 21                	jae    464bb1 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x421>
  464b90:	8b 4c 8f 04          	mov    0x4(%rdi,%rcx,4),%ecx
  464b94:	48 c1 e1 08          	shl    $0x8,%rcx
  464b98:	0f 18 0c 0e          	prefetcht0 (%rsi,%rcx,1)
  464b9c:	0f 18 4c 0e 40       	prefetcht0 0x40(%rsi,%rcx,1)
  464ba1:	0f 18 8c 0e 80 00 00 	prefetcht0 0x80(%rsi,%rcx,1)
  464ba8:	00 
  464ba9:	0f 18 8c 0e c0 00 00 	prefetcht0 0xc0(%rsi,%rcx,1)
  464bb0:	00 
  464bb1:	44 89 e1             	mov    %r12d,%ecx
  464bb4:	80 e1 03             	and    $0x3,%cl
  464bb7:	80 f9 02             	cmp    $0x2,%cl
  464bba:	0f 82 10 01 00 00    	jb     464cd0 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x540>
  464bc0:	45 8d 56 01          	lea    0x1(%r14),%r10d
  464bc4:	41 39 da             	cmp    %ebx,%r10d
  464bc7:	73 47                	jae    464c10 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x480>
  464bc9:	4c 8b 44 24 c8       	mov    -0x38(%rsp),%r8
  464bce:	47 8b 14 98          	mov    (%r8,%r11,4),%r10d
  464bd2:	4d 89 d8             	mov    %r11,%r8
  464bd5:	45 89 eb             	mov    %r13d,%r11d
  464bd8:	41 89 dd             	mov    %ebx,%r13d
  464bdb:	44 89 f3             	mov    %r14d,%ebx
  464bde:	4c 01 d3             	add    %r10,%rbx
  464be1:	44 8b 54 9f 04       	mov    0x4(%rdi,%rbx,4),%r10d
  464be6:	44 89 eb             	mov    %r13d,%ebx
  464be9:	45 89 dd             	mov    %r11d,%r13d
  464bec:	4d 89 c3             	mov    %r8,%r11
  464bef:	49 c1 e2 08          	shl    $0x8,%r10
  464bf3:	42 0f 18 0c 16       	prefetcht0 (%rsi,%r10,1)
  464bf8:	42 0f 18 4c 16 40    	prefetcht0 0x40(%rsi,%r10,1)
  464bfe:	42 0f 18 8c 16 80 00 	prefetcht0 0x80(%rsi,%r10,1)
  464c05:	00 00 
  464c07:	42 0f 18 8c 16 c0 00 	prefetcht0 0xc0(%rsi,%r10,1)
  464c0e:	00 00 
  464c10:	80 f9 03             	cmp    $0x3,%cl
  464c13:	0f 85 67 fe ff ff    	jne    464a80 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x2f0>
  464c19:	45 89 ca             	mov    %r9d,%r10d
  464c1c:	49 c1 e2 08          	shl    $0x8,%r10
  464c20:	62 a2 7d 48 33 44 16 	vpmovzxwd 0xa0(%rsi,%r10,1),%zmm16
  464c27:	05 
  464c28:	62 a2 7d 48 33 4c 16 	vpmovzxwd 0x80(%rsi,%r10,1),%zmm17
  464c2f:	04 
  464c30:	62 a2 7d 48 33 54 16 	vpmovzxwd 0x60(%rsi,%r10,1),%zmm18
  464c37:	03 
  464c38:	62 a2 7d 48 33 5c 16 	vpmovzxwd 0x40(%rsi,%r10,1),%zmm19
  464c3f:	02 
  464c40:	62 a2 7d 48 33 64 16 	vpmovzxwd 0x20(%rsi,%r10,1),%zmm20
  464c47:	01 
  464c48:	62 a2 7d 48 33 2c 16 	vpmovzxwd (%rsi,%r10,1),%zmm21
  464c4f:	62 a2 7d 48 33 74 16 	vpmovzxwd 0xc0(%rsi,%r10,1),%zmm22
  464c56:	06 
  464c57:	62 a2 7d 48 33 7c 16 	vpmovzxwd 0xe0(%rsi,%r10,1),%zmm23
  464c5e:	07 
  464c5f:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  464c66:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  464c6d:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  464c74:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  464c7b:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  464c82:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  464c89:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  464c90:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  464c97:	62 b1 5c 48 58 e0    	vaddps %zmm16,%zmm4,%zmm4
  464c9d:	62 b1 4c 48 58 f1    	vaddps %zmm17,%zmm6,%zmm6
  464ca3:	62 31 34 48 58 ca    	vaddps %zmm18,%zmm9,%zmm9
  464ca9:	62 31 1c 48 58 e3    	vaddps %zmm19,%zmm12,%zmm12
  464caf:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  464cb5:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  464cbb:	62 b1 6c 48 58 d6    	vaddps %zmm22,%zmm2,%zmm2
  464cc1:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  464cc7:	e9 32 fe ff ff       	jmpq   464afe <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x36e>
  464ccc:	0f 1f 40 00          	nopl   0x0(%rax)
  464cd0:	80 f9 01             	cmp    $0x1,%cl
  464cd3:	0f 85 a7 fd ff ff    	jne    464a80 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x2f0>
  464cd9:	45 89 ca             	mov    %r9d,%r10d
  464cdc:	49 c1 e2 08          	shl    $0x8,%r10
  464ce0:	62 a2 7d 48 33 44 16 	vpmovzxwd 0xa0(%rsi,%r10,1),%zmm16
  464ce7:	05 
  464ce8:	62 a2 7d 48 33 4c 16 	vpmovzxwd 0x80(%rsi,%r10,1),%zmm17
  464cef:	04 
  464cf0:	62 a2 7d 48 33 54 16 	vpmovzxwd 0x60(%rsi,%r10,1),%zmm18
  464cf7:	03 
  464cf8:	62 a2 7d 48 33 5c 16 	vpmovzxwd 0x40(%rsi,%r10,1),%zmm19
  464cff:	02 
  464d00:	62 a2 7d 48 33 64 16 	vpmovzxwd 0x20(%rsi,%r10,1),%zmm20
  464d07:	01 
  464d08:	62 a2 7d 48 33 2c 16 	vpmovzxwd (%rsi,%r10,1),%zmm21
  464d0f:	62 a2 7d 48 33 74 16 	vpmovzxwd 0xc0(%rsi,%r10,1),%zmm22
  464d16:	06 
  464d17:	62 a2 7d 48 33 7c 16 	vpmovzxwd 0xe0(%rsi,%r10,1),%zmm23
  464d1e:	07 
  464d1f:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  464d26:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  464d2d:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  464d34:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  464d3b:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  464d42:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  464d49:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  464d50:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  464d57:	62 b1 5c 48 58 e0    	vaddps %zmm16,%zmm4,%zmm4
  464d5d:	62 b1 4c 48 58 f1    	vaddps %zmm17,%zmm6,%zmm6
  464d63:	62 31 34 48 58 ca    	vaddps %zmm18,%zmm9,%zmm9
  464d69:	62 31 1c 48 58 e3    	vaddps %zmm19,%zmm12,%zmm12
  464d6f:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  464d75:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  464d7b:	62 b1 6c 48 58 d6    	vaddps %zmm22,%zmm2,%zmm2
  464d81:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  464d87:	e9 a2 fd ff ff       	jmpq   464b2e <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x39e>
  464d8c:	48 8b 4c 24 70       	mov    0x70(%rsp),%rcx
  464d91:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  464d96:	48 8b 54 24 60       	mov    0x60(%rsp),%rdx
  464d9b:	80 7c 24 bb 00       	cmpb   $0x0,-0x45(%rsp)
  464da0:	8b 7c 24 d4          	mov    -0x2c(%rsp),%edi
  464da4:	4c 8b 44 24 58       	mov    0x58(%rsp),%r8
  464da9:	0f 84 bd 01 00 00    	je     464f6c <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x7dc>
  464daf:	80 79 2a 00          	cmpb   $0x0,0x2a(%rcx)
  464db3:	0f 84 7f 03 00 00    	je     465138 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x9a8>
  464db9:	44 3b 6c 24 40       	cmp    0x40(%rsp),%r13d
  464dbe:	0f 87 74 03 00 00    	ja     465138 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x9a8>
  464dc4:	62 d1 7d 40 72 d5 10 	vpsrld $0x10,%zmm13,%zmm16
  464dcb:	8b 74 24 c4          	mov    -0x3c(%rsp),%esi
  464dcf:	c1 e6 07             	shl    $0x7,%esi
  464dd2:	48 63 f6             	movslq %esi,%rsi
  464dd5:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464dd9:	62 e2 7e 48 33 04 72 	vpmovdw %zmm16,(%rdx,%rsi,2)
  464de0:	74 1e                	je     464e00 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x670>
  464de2:	62 e1 14 58 54 05 0c 	vandps 0x40d0c(%rip){1to16},%zmm13,%zmm16        # 4a5af8 <__dso_handle+0x30>
  464de9:	0d 04 00 
  464dec:	62 31 14 48 5c e8    	vsubps %zmm16,%zmm13,%zmm13
  464df2:	62 d1 15 48 72 d5 10 	vpsrld $0x10,%zmm13,%zmm13
  464df9:	62 72 7e 48 33 2c 70 	vpmovdw %zmm13,(%rax,%rsi,2)
  464e00:	62 d1 15 48 72 d7 10 	vpsrld $0x10,%zmm15,%zmm13
  464e07:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464e0b:	62 72 7e 48 33 6c 72 	vpmovdw %zmm13,0x20(%rdx,%rsi,2)
  464e12:	01 
  464e13:	74 1f                	je     464e34 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x6a4>
  464e15:	62 71 04 58 54 2d d9 	vandps 0x40cd9(%rip){1to16},%zmm15,%zmm13        # 4a5af8 <__dso_handle+0x30>
  464e1c:	0c 04 00 
  464e1f:	62 51 04 48 5c ed    	vsubps %zmm13,%zmm15,%zmm13
  464e25:	62 d1 15 48 72 d5 10 	vpsrld $0x10,%zmm13,%zmm13
  464e2c:	62 72 7e 48 33 6c 70 	vpmovdw %zmm13,0x20(%rax,%rsi,2)
  464e33:	01 
  464e34:	62 d1 15 48 72 d4 10 	vpsrld $0x10,%zmm12,%zmm13
  464e3b:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464e3f:	62 72 7e 48 33 6c 72 	vpmovdw %zmm13,0x40(%rdx,%rsi,2)
  464e46:	02 
  464e47:	74 1f                	je     464e68 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x6d8>
  464e49:	62 71 1c 58 54 2d a5 	vandps 0x40ca5(%rip){1to16},%zmm12,%zmm13        # 4a5af8 <__dso_handle+0x30>
  464e50:	0c 04 00 
  464e53:	62 51 1c 48 5c e5    	vsubps %zmm13,%zmm12,%zmm12
  464e59:	62 d1 1d 48 72 d4 10 	vpsrld $0x10,%zmm12,%zmm12
  464e60:	62 72 7e 48 33 64 70 	vpmovdw %zmm12,0x40(%rax,%rsi,2)
  464e67:	02 
  464e68:	62 d1 1d 48 72 d1 10 	vpsrld $0x10,%zmm9,%zmm12
  464e6f:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464e73:	62 72 7e 48 33 64 72 	vpmovdw %zmm12,0x60(%rdx,%rsi,2)
  464e7a:	03 
  464e7b:	74 1f                	je     464e9c <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x70c>
  464e7d:	62 71 34 58 54 25 71 	vandps 0x40c71(%rip){1to16},%zmm9,%zmm12        # 4a5af8 <__dso_handle+0x30>
  464e84:	0c 04 00 
  464e87:	62 51 34 48 5c cc    	vsubps %zmm12,%zmm9,%zmm9
  464e8d:	62 d1 35 48 72 d1 10 	vpsrld $0x10,%zmm9,%zmm9
  464e94:	62 72 7e 48 33 4c 70 	vpmovdw %zmm9,0x60(%rax,%rsi,2)
  464e9b:	03 
  464e9c:	62 f1 35 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm9
  464ea3:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464ea7:	62 72 7e 48 33 4c 72 	vpmovdw %zmm9,0x80(%rdx,%rsi,2)
  464eae:	04 
  464eaf:	74 1f                	je     464ed0 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x740>
  464eb1:	62 71 4c 58 54 0d 3d 	vandps 0x40c3d(%rip){1to16},%zmm6,%zmm9        # 4a5af8 <__dso_handle+0x30>
  464eb8:	0c 04 00 
  464ebb:	62 d1 4c 48 5c f1    	vsubps %zmm9,%zmm6,%zmm6
  464ec1:	62 f1 4d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm6
  464ec8:	62 f2 7e 48 33 74 70 	vpmovdw %zmm6,0x80(%rax,%rsi,2)
  464ecf:	04 
  464ed0:	62 f1 4d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm6
  464ed7:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464edb:	62 f2 7e 48 33 74 72 	vpmovdw %zmm6,0xa0(%rdx,%rsi,2)
  464ee2:	05 
  464ee3:	74 1f                	je     464f04 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x774>
  464ee5:	62 f1 5c 58 54 35 09 	vandps 0x40c09(%rip){1to16},%zmm4,%zmm6        # 4a5af8 <__dso_handle+0x30>
  464eec:	0c 04 00 
  464eef:	62 f1 5c 48 5c e6    	vsubps %zmm6,%zmm4,%zmm4
  464ef5:	62 f1 5d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm4
  464efc:	62 f2 7e 48 33 64 70 	vpmovdw %zmm4,0xa0(%rax,%rsi,2)
  464f03:	05 
  464f04:	62 f1 5d 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm4
  464f0b:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464f0f:	62 f2 7e 48 33 64 72 	vpmovdw %zmm4,0xc0(%rdx,%rsi,2)
  464f16:	06 
  464f17:	74 1f                	je     464f38 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x7a8>
  464f19:	62 f1 6c 58 54 25 d5 	vandps 0x40bd5(%rip){1to16},%zmm2,%zmm4        # 4a5af8 <__dso_handle+0x30>
  464f20:	0b 04 00 
  464f23:	62 f1 6c 48 5c d4    	vsubps %zmm4,%zmm2,%zmm2
  464f29:	62 f1 6d 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm2
  464f30:	62 f2 7e 48 33 54 70 	vpmovdw %zmm2,0xc0(%rax,%rsi,2)
  464f37:	06 
  464f38:	62 f1 6d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm2
  464f3f:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464f43:	62 f2 7e 48 33 54 72 	vpmovdw %zmm2,0xe0(%rdx,%rsi,2)
  464f4a:	07 
  464f4b:	74 1f                	je     464f6c <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x7dc>
  464f4d:	62 f1 7c 58 54 15 a1 	vandps 0x40ba1(%rip){1to16},%zmm0,%zmm2        # 4a5af8 <__dso_handle+0x30>
  464f54:	0b 04 00 
  464f57:	62 f1 7c 48 5c c2    	vsubps %zmm2,%zmm0,%zmm0
  464f5d:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  464f64:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0xe0(%rax,%rsi,2)
  464f6b:	07 
  464f6c:	80 7c 24 bc 00       	cmpb   $0x0,-0x44(%rsp)
  464f71:	0f 84 5c 02 00 00    	je     4651d3 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa43>
  464f77:	80 79 2a 00          	cmpb   $0x0,0x2a(%rcx)
  464f7b:	0f 84 0d 02 00 00    	je     46518e <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x9fe>
  464f81:	39 5c 24 c0          	cmp    %ebx,-0x40(%rsp)
  464f85:	0f 85 03 02 00 00    	jne    46518e <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x9fe>
  464f8b:	62 d1 7d 48 72 d2 10 	vpsrld $0x10,%zmm10,%zmm0
  464f92:	c1 e7 07             	shl    $0x7,%edi
  464f95:	48 63 f7             	movslq %edi,%rsi
  464f98:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464f9c:	62 f2 7e 48 33 04 72 	vpmovdw %zmm0,(%rdx,%rsi,2)
  464fa3:	74 1e                	je     464fc3 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x833>
  464fa5:	62 f1 2c 58 54 05 49 	vandps 0x40b49(%rip){1to16},%zmm10,%zmm0        # 4a5af8 <__dso_handle+0x30>
  464fac:	0b 04 00 
  464faf:	62 f1 2c 48 5c c0    	vsubps %zmm0,%zmm10,%zmm0
  464fb5:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  464fbc:	62 f2 7e 48 33 04 70 	vpmovdw %zmm0,(%rax,%rsi,2)
  464fc3:	62 d1 7d 48 72 d6 10 	vpsrld $0x10,%zmm14,%zmm0
  464fca:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464fce:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0x20(%rdx,%rsi,2)
  464fd5:	01 
  464fd6:	74 1f                	je     464ff7 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x867>
  464fd8:	62 f1 0c 58 54 05 16 	vandps 0x40b16(%rip){1to16},%zmm14,%zmm0        # 4a5af8 <__dso_handle+0x30>
  464fdf:	0b 04 00 
  464fe2:	62 f1 0c 48 5c c0    	vsubps %zmm0,%zmm14,%zmm0
  464fe8:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  464fef:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0x20(%rax,%rsi,2)
  464ff6:	01 
  464ff7:	62 d1 7d 48 72 d3 10 	vpsrld $0x10,%zmm11,%zmm0
  464ffe:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  465002:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0x40(%rdx,%rsi,2)
  465009:	02 
  46500a:	74 1f                	je     46502b <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x89b>
  46500c:	62 f1 24 58 54 05 e2 	vandps 0x40ae2(%rip){1to16},%zmm11,%zmm0        # 4a5af8 <__dso_handle+0x30>
  465013:	0a 04 00 
  465016:	62 f1 24 48 5c c0    	vsubps %zmm0,%zmm11,%zmm0
  46501c:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  465023:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0x40(%rax,%rsi,2)
  46502a:	02 
  46502b:	62 d1 7d 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm0
  465032:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  465036:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0x60(%rdx,%rsi,2)
  46503d:	03 
  46503e:	74 1f                	je     46505f <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x8cf>
  465040:	62 f1 3c 58 54 05 ae 	vandps 0x40aae(%rip){1to16},%zmm8,%zmm0        # 4a5af8 <__dso_handle+0x30>
  465047:	0a 04 00 
  46504a:	62 f1 3c 48 5c c0    	vsubps %zmm0,%zmm8,%zmm0
  465050:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  465057:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0x60(%rax,%rsi,2)
  46505e:	03 
  46505f:	62 f1 7d 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm0
  465066:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  46506a:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0x80(%rdx,%rsi,2)
  465071:	04 
  465072:	74 1f                	je     465093 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x903>
  465074:	62 f1 44 58 54 05 7a 	vandps 0x40a7a(%rip){1to16},%zmm7,%zmm0        # 4a5af8 <__dso_handle+0x30>
  46507b:	0a 04 00 
  46507e:	62 f1 44 48 5c c0    	vsubps %zmm0,%zmm7,%zmm0
  465084:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46508b:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0x80(%rax,%rsi,2)
  465092:	04 
  465093:	62 f1 7d 48 72 d5 10 	vpsrld $0x10,%zmm5,%zmm0
  46509a:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  46509e:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0xa0(%rdx,%rsi,2)
  4650a5:	05 
  4650a6:	74 1f                	je     4650c7 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x937>
  4650a8:	62 f1 54 58 54 05 46 	vandps 0x40a46(%rip){1to16},%zmm5,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4650af:	0a 04 00 
  4650b2:	62 f1 54 48 5c c0    	vsubps %zmm0,%zmm5,%zmm0
  4650b8:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4650bf:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0xa0(%rax,%rsi,2)
  4650c6:	05 
  4650c7:	62 f1 7d 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm0
  4650ce:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  4650d2:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0xc0(%rdx,%rsi,2)
  4650d9:	06 
  4650da:	74 1f                	je     4650fb <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x96b>
  4650dc:	62 f1 64 58 54 05 12 	vandps 0x40a12(%rip){1to16},%zmm3,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4650e3:	0a 04 00 
  4650e6:	62 f1 64 48 5c c0    	vsubps %zmm0,%zmm3,%zmm0
  4650ec:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4650f3:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0xc0(%rax,%rsi,2)
  4650fa:	06 
  4650fb:	62 f1 7d 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm0
  465102:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  465106:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0xe0(%rdx,%rsi,2)
  46510d:	07 
  46510e:	0f 84 bf 00 00 00    	je     4651d3 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa43>
  465114:	62 f1 74 58 54 05 da 	vandps 0x409da(%rip){1to16},%zmm1,%zmm0        # 4a5af8 <__dso_handle+0x30>
  46511b:	09 04 00 
  46511e:	62 f1 74 48 5c c0    	vsubps %zmm0,%zmm1,%zmm0
  465124:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46512b:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0xe0(%rax,%rsi,2)
  465132:	07 
  465133:	e9 9b 00 00 00       	jmpq   4651d3 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa43>
  465138:	8b 74 24 c4          	mov    -0x3c(%rsp),%esi
  46513c:	c1 e6 07             	shl    $0x7,%esi
  46513f:	48 63 f6             	movslq %esi,%rsi
  465142:	62 51 7c 48 11 64 b0 	vmovups %zmm12,0x80(%r8,%rsi,4)
  465149:	02 
  46514a:	62 51 7c 48 11 7c b0 	vmovups %zmm15,0x40(%r8,%rsi,4)
  465151:	01 
  465152:	62 51 7c 48 11 2c b0 	vmovups %zmm13,(%r8,%rsi,4)
  465159:	62 51 7c 48 11 4c b0 	vmovups %zmm9,0xc0(%r8,%rsi,4)
  465160:	03 
  465161:	62 d1 7c 48 11 74 b0 	vmovups %zmm6,0x100(%r8,%rsi,4)
  465168:	04 
  465169:	62 d1 7c 48 11 64 b0 	vmovups %zmm4,0x140(%r8,%rsi,4)
  465170:	05 
  465171:	62 d1 7c 48 11 54 b0 	vmovups %zmm2,0x180(%r8,%rsi,4)
  465178:	06 
  465179:	62 d1 7c 48 11 44 b0 	vmovups %zmm0,0x1c0(%r8,%rsi,4)
  465180:	07 
  465181:	80 7c 24 bc 00       	cmpb   $0x0,-0x44(%rsp)
  465186:	0f 85 eb fd ff ff    	jne    464f77 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x7e7>
  46518c:	eb 45                	jmp    4651d3 <void gcn_extra_event_trace::trace_pair_window<false, 2>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa43>
  46518e:	c1 e7 07             	shl    $0x7,%edi
  465191:	48 63 c7             	movslq %edi,%rax
  465194:	62 51 7c 48 11 5c 80 	vmovups %zmm11,0x80(%r8,%rax,4)
  46519b:	02 
  46519c:	62 51 7c 48 11 74 80 	vmovups %zmm14,0x40(%r8,%rax,4)
  4651a3:	01 
  4651a4:	62 51 7c 48 11 14 80 	vmovups %zmm10,(%r8,%rax,4)
  4651ab:	62 51 7c 48 11 44 80 	vmovups %zmm8,0xc0(%r8,%rax,4)
  4651b2:	03 
  4651b3:	62 d1 7c 48 11 7c 80 	vmovups %zmm7,0x100(%r8,%rax,4)
  4651ba:	04 
  4651bb:	62 d1 7c 48 11 6c 80 	vmovups %zmm5,0x140(%r8,%rax,4)
  4651c2:	05 
  4651c3:	62 d1 7c 48 11 5c 80 	vmovups %zmm3,0x180(%r8,%rax,4)
  4651ca:	06 
  4651cb:	62 d1 7c 48 11 4c 80 	vmovups %zmm1,0x1c0(%r8,%rax,4)
  4651d2:	07 
  4651d3:	5b                   	pop    %rbx
  4651d4:	41 5c                	pop    %r12
  4651d6:	41 5d                	pop    %r13
  4651d8:	41 5e                	pop    %r14
  4651da:	41 5f                	pop    %r15
  4651dc:	5d                   	pop    %rbp
  4651dd:	c5 f8 77             	vzeroupper 
  4651e0:	c3                   	retq   
  4651e1:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
  4651e8:	00 00 00 
  4651eb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

