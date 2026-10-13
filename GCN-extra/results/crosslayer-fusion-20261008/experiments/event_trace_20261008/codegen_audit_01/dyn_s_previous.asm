000000000046ae00 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)>:
  46ae00:	55                   	push   %rbp
  46ae01:	41 57                	push   %r15
  46ae03:	41 56                	push   %r14
  46ae05:	41 55                	push   %r13
  46ae07:	41 54                	push   %r12
  46ae09:	53                   	push   %rbx
  46ae0a:	48 89 54 24 d8       	mov    %rdx,-0x28(%rsp)
  46ae0f:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  46ae14:	44 0f b6 5c 24 50    	movzbl 0x50(%rsp),%r11d
  46ae1a:	8b 6c 24 48          	mov    0x48(%rsp),%ebp
  46ae1e:	44 8b 74 24 40       	mov    0x40(%rsp),%r14d
  46ae23:	44 8b 7c 24 38       	mov    0x38(%rsp),%r15d
  46ae28:	49 63 c0             	movslq %r8d,%rax
  46ae2b:	48 89 44 24 f0       	mov    %rax,-0x10(%rsp)
  46ae30:	8b 04 81             	mov    (%rcx,%rax,4),%eax
  46ae33:	39 e8                	cmp    %ebp,%eax
  46ae35:	0f 43 c5             	cmovae %ebp,%eax
  46ae38:	44 39 f0             	cmp    %r14d,%eax
  46ae3b:	44 89 f2             	mov    %r14d,%edx
  46ae3e:	89 44 24 ec          	mov    %eax,-0x14(%rsp)
  46ae42:	0f 42 d0             	cmovb  %eax,%edx
  46ae45:	44 39 fa             	cmp    %r15d,%edx
  46ae48:	0f 97 c0             	seta   %al
  46ae4b:	45 89 dc             	mov    %r11d,%r12d
  46ae4e:	41 08 c4             	or     %al,%r12b
  46ae51:	45 85 c9             	test   %r9d,%r9d
  46ae54:	0f 88 89 00 00 00    	js     46aee3 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xe3>
  46ae5a:	45 89 ca             	mov    %r9d,%r10d
  46ae5d:	42 8b 0c 91          	mov    (%rcx,%r10,4),%ecx
  46ae61:	39 e9                	cmp    %ebp,%ecx
  46ae63:	0f 42 e9             	cmovb  %ecx,%ebp
  46ae66:	44 39 f5             	cmp    %r14d,%ebp
  46ae69:	44 89 f1             	mov    %r14d,%ecx
  46ae6c:	41 89 ed             	mov    %ebp,%r13d
  46ae6f:	0f 42 cd             	cmovb  %ebp,%ecx
  46ae72:	44 39 f9             	cmp    %r15d,%ecx
  46ae75:	40 0f 97 c5          	seta   %bpl
  46ae79:	44 08 dd             	or     %r11b,%bpl
  46ae7c:	45 84 e4             	test   %r12b,%r12b
  46ae7f:	0f 84 bc 00 00 00    	je     46af41 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x141>
  46ae85:	45 84 db             	test   %r11b,%r11b
  46ae88:	0f 84 c0 00 00 00    	je     46af4e <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x14e>
  46ae8e:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  46ae93:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  46ae97:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  46ae9b:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  46ae9f:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  46aea3:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  46aea7:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  46aeab:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  46aeaf:	40 84 ed             	test   %bpl,%bpl
  46aeb2:	0f 85 e8 00 00 00    	jne    46afa0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x1a0>
  46aeb8:	44 89 44 24 e4       	mov    %r8d,-0x1c(%rsp)
  46aebd:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  46aec2:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  46aec9:	00 
  46aeca:	89 c5                	mov    %eax,%ebp
  46aecc:	45 31 db             	xor    %r11d,%r11d
  46aecf:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  46aed5:	40 84 ed             	test   %bpl,%bpl
  46aed8:	0f 85 69 01 00 00    	jne    46b047 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x247>
  46aede:	e9 80 01 00 00       	jmpq   46b063 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x263>
  46aee3:	45 84 e4             	test   %r12b,%r12b
  46aee6:	0f 84 57 0a 00 00    	je     46b943 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  46aeec:	45 84 db             	test   %r11b,%r11b
  46aeef:	0f 84 bd 01 00 00    	je     46b0b2 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x2b2>
  46aef5:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  46aefa:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  46aefe:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  46af02:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  46af06:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  46af0a:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  46af0e:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  46af12:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  46af16:	44 89 44 24 e4       	mov    %r8d,-0x1c(%rsp)
  46af1b:	44 39 fa             	cmp    %r15d,%edx
  46af1e:	0f 87 e4 01 00 00    	ja     46b108 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x308>
  46af24:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  46af29:	45 31 ed             	xor    %r13d,%r13d
  46af2c:	45 31 e4             	xor    %r12d,%r12d
  46af2f:	31 c9                	xor    %ecx,%ecx
  46af31:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  46af38:	00 
  46af39:	45 31 db             	xor    %r11d,%r11d
  46af3c:	e9 3e 01 00 00       	jmpq   46b07f <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x27f>
  46af41:	40 84 ed             	test   %bpl,%bpl
  46af44:	0f 84 f9 09 00 00    	je     46b943 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  46af4a:	31 ed                	xor    %ebp,%ebp
  46af4c:	eb 54                	jmp    46afa2 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x1a2>
  46af4e:	45 89 c6             	mov    %r8d,%r14d
  46af51:	41 c1 e6 07          	shl    $0x7,%r14d
  46af55:	4d 63 f6             	movslq %r14d,%r14
  46af58:	62 b1 7c 48 10 44 b3 	vmovups 0x1c0(%rbx,%r14,4),%zmm0
  46af5f:	07 
  46af60:	62 b1 7c 48 10 4c b3 	vmovups 0x180(%rbx,%r14,4),%zmm1
  46af67:	06 
  46af68:	62 b1 7c 48 10 54 b3 	vmovups 0x140(%rbx,%r14,4),%zmm2
  46af6f:	05 
  46af70:	62 b1 7c 48 10 5c b3 	vmovups 0x100(%rbx,%r14,4),%zmm3
  46af77:	04 
  46af78:	62 31 7c 48 10 04 b3 	vmovups (%rbx,%r14,4),%zmm8
  46af7f:	62 b1 7c 48 10 7c b3 	vmovups 0x40(%rbx,%r14,4),%zmm7
  46af86:	01 
  46af87:	62 b1 7c 48 10 74 b3 	vmovups 0x80(%rbx,%r14,4),%zmm6
  46af8e:	02 
  46af8f:	62 b1 7c 48 10 64 b3 	vmovups 0xc0(%rbx,%r14,4),%zmm4
  46af96:	03 
  46af97:	40 84 ed             	test   %bpl,%bpl
  46af9a:	0f 84 18 ff ff ff    	je     46aeb8 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb8>
  46afa0:	89 c5                	mov    %eax,%ebp
  46afa2:	41 b6 01             	mov    $0x1,%r14b
  46afa5:	44 89 74 24 d4       	mov    %r14d,-0x2c(%rsp)
  46afaa:	45 84 db             	test   %r11b,%r11b
  46afad:	44 89 44 24 e4       	mov    %r8d,-0x1c(%rsp)
  46afb2:	44 88 64 24 d3       	mov    %r12b,-0x2d(%rsp)
  46afb7:	74 37                	je     46aff0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x1f0>
  46afb9:	c4 41 10 57 ed       	vxorps %xmm13,%xmm13,%xmm13
  46afbe:	c4 41 00 57 ff       	vxorps %xmm15,%xmm15,%xmm15
  46afc3:	c4 41 08 57 f6       	vxorps %xmm14,%xmm14,%xmm14
  46afc8:	c4 41 18 57 e4       	vxorps %xmm12,%xmm12,%xmm12
  46afcd:	c4 41 20 57 db       	vxorps %xmm11,%xmm11,%xmm11
  46afd2:	c4 41 28 57 d2       	vxorps %xmm10,%xmm10,%xmm10
  46afd7:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  46afdc:	c5 d0 57 ed          	vxorps %xmm5,%xmm5,%xmm5
  46afe0:	45 31 db             	xor    %r11d,%r11d
  46afe3:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  46afe9:	40 84 ed             	test   %bpl,%bpl
  46afec:	75 59                	jne    46b047 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x247>
  46afee:	eb 73                	jmp    46b063 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x263>
  46aff0:	45 89 cb             	mov    %r9d,%r11d
  46aff3:	41 c1 e3 07          	shl    $0x7,%r11d
  46aff7:	4d 63 db             	movslq %r11d,%r11
  46affa:	62 b1 7c 48 10 6c 9b 	vmovups 0x1c0(%rbx,%r11,4),%zmm5
  46b001:	07 
  46b002:	62 31 7c 48 10 4c 9b 	vmovups 0x180(%rbx,%r11,4),%zmm9
  46b009:	06 
  46b00a:	62 31 7c 48 10 54 9b 	vmovups 0x140(%rbx,%r11,4),%zmm10
  46b011:	05 
  46b012:	62 31 7c 48 10 5c 9b 	vmovups 0x100(%rbx,%r11,4),%zmm11
  46b019:	04 
  46b01a:	62 31 7c 48 10 2c 9b 	vmovups (%rbx,%r11,4),%zmm13
  46b021:	62 31 7c 48 10 7c 9b 	vmovups 0x40(%rbx,%r11,4),%zmm15
  46b028:	01 
  46b029:	62 31 7c 48 10 74 9b 	vmovups 0x80(%rbx,%r11,4),%zmm14
  46b030:	02 
  46b031:	62 31 7c 48 10 64 9b 	vmovups 0xc0(%rbx,%r11,4),%zmm12
  46b038:	03 
  46b039:	45 31 db             	xor    %r11d,%r11d
  46b03c:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  46b042:	40 84 ed             	test   %bpl,%bpl
  46b045:	74 1c                	je     46b063 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x263>
  46b047:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46b04c:	4c 8b 74 24 f0       	mov    -0x10(%rsp),%r14
  46b051:	47 8b 34 b0          	mov    (%r8,%r14,4),%r14d
  46b055:	45 89 fc             	mov    %r15d,%r12d
  46b058:	4d 01 f4             	add    %r14,%r12
  46b05b:	4c 8b 77 18          	mov    0x18(%rdi),%r14
  46b05f:	47 8b 24 a6          	mov    (%r14,%r12,4),%r12d
  46b063:	44 39 f9             	cmp    %r15d,%ecx
  46b066:	76 17                	jbe    46b07f <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x27f>
  46b068:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46b06d:	47 8b 14 90          	mov    (%r8,%r10,4),%r10d
  46b071:	45 89 fb             	mov    %r15d,%r11d
  46b074:	4d 01 d3             	add    %r10,%r11
  46b077:	4c 8b 57 18          	mov    0x18(%rdi),%r10
  46b07b:	47 8b 1c 9a          	mov    (%r10,%r11,4),%r11d
  46b07f:	45 85 c9             	test   %r9d,%r9d
  46b082:	40 0f 99 c5          	setns  %bpl
  46b086:	44 39 f9             	cmp    %r15d,%ecx
  46b089:	41 0f 97 c2          	seta   %r10b
  46b08d:	41 20 ea             	and    %bpl,%r10b
  46b090:	44 39 fa             	cmp    %r15d,%edx
  46b093:	77 11                	ja     46b0a6 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x2a6>
  46b095:	45 84 d2             	test   %r10b,%r10b
  46b098:	75 0c                	jne    46b0a6 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x2a6>
  46b09a:	89 4c 24 e8          	mov    %ecx,-0x18(%rsp)
  46b09e:	44 89 ed             	mov    %r13d,%ebp
  46b0a1:	e9 58 04 00 00       	jmpq   46b4fe <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6fe>
  46b0a6:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  46b0aa:	44 89 ed             	mov    %r13d,%ebp
  46b0ad:	e9 89 00 00 00       	jmpq   46b13b <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x33b>
  46b0b2:	44 89 c1             	mov    %r8d,%ecx
  46b0b5:	c1 e1 07             	shl    $0x7,%ecx
  46b0b8:	48 63 c9             	movslq %ecx,%rcx
  46b0bb:	62 f1 7c 48 10 44 8b 	vmovups 0x1c0(%rbx,%rcx,4),%zmm0
  46b0c2:	07 
  46b0c3:	62 f1 7c 48 10 4c 8b 	vmovups 0x180(%rbx,%rcx,4),%zmm1
  46b0ca:	06 
  46b0cb:	62 f1 7c 48 10 54 8b 	vmovups 0x140(%rbx,%rcx,4),%zmm2
  46b0d2:	05 
  46b0d3:	62 f1 7c 48 10 5c 8b 	vmovups 0x100(%rbx,%rcx,4),%zmm3
  46b0da:	04 
  46b0db:	62 71 7c 48 10 04 8b 	vmovups (%rbx,%rcx,4),%zmm8
  46b0e2:	62 f1 7c 48 10 7c 8b 	vmovups 0x40(%rbx,%rcx,4),%zmm7
  46b0e9:	01 
  46b0ea:	62 f1 7c 48 10 74 8b 	vmovups 0x80(%rbx,%rcx,4),%zmm6
  46b0f1:	02 
  46b0f2:	62 f1 7c 48 10 64 8b 	vmovups 0xc0(%rbx,%rcx,4),%zmm4
  46b0f9:	03 
  46b0fa:	44 89 44 24 e4       	mov    %r8d,-0x1c(%rsp)
  46b0ff:	44 39 fa             	cmp    %r15d,%edx
  46b102:	0f 86 1c fe ff ff    	jbe    46af24 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x124>
  46b108:	48 8b 4c 24 d8       	mov    -0x28(%rsp),%rcx
  46b10d:	4c 8b 44 24 f0       	mov    -0x10(%rsp),%r8
  46b112:	42 8b 0c 81          	mov    (%rcx,%r8,4),%ecx
  46b116:	45 89 fa             	mov    %r15d,%r10d
  46b119:	49 01 ca             	add    %rcx,%r10
  46b11c:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  46b120:	46 8b 24 97          	mov    (%rdi,%r10,4),%r12d
  46b124:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  46b129:	45 31 d2             	xor    %r10d,%r10d
  46b12c:	45 31 db             	xor    %r11d,%r11d
  46b12f:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  46b136:	00 
  46b137:	31 c9                	xor    %ecx,%ecx
  46b139:	31 ed                	xor    %ebp,%ebp
  46b13b:	4d 63 c1             	movslq %r9d,%r8
  46b13e:	4c 89 44 24 f8       	mov    %r8,-0x8(%rsp)
  46b143:	45 89 fd             	mov    %r15d,%r13d
  46b146:	89 4c 24 e8          	mov    %ecx,-0x18(%rsp)
  46b14a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
  46b150:	a8 01                	test   $0x1,%al
  46b152:	74 2c                	je     46b180 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x380>
  46b154:	45 39 e3             	cmp    %r12d,%r11d
  46b157:	b8 03 00 00 00       	mov    $0x3,%eax
  46b15c:	83 d8 00             	sbb    $0x0,%eax
  46b15f:	41 f6 c2 01          	test   $0x1,%r10b
  46b163:	41 b8 01 00 00 00    	mov    $0x1,%r8d
  46b169:	41 0f 44 c0          	cmove  %r8d,%eax
  46b16d:	45 39 dc             	cmp    %r11d,%r12d
  46b170:	41 0f 42 c0          	cmovb  %r8d,%eax
  46b174:	45 8d 57 01          	lea    0x1(%r15),%r10d
  46b178:	a8 01                	test   $0x1,%al
  46b17a:	75 15                	jne    46b191 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x391>
  46b17c:	eb 4f                	jmp    46b1cd <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3cd>
  46b17e:	66 90                	xchg   %ax,%ax
  46b180:	41 0f b6 c2          	movzbl %r10b,%eax
  46b184:	83 e0 01             	and    $0x1,%eax
  46b187:	01 c0                	add    %eax,%eax
  46b189:	45 8d 57 01          	lea    0x1(%r15),%r10d
  46b18d:	a8 01                	test   $0x1,%al
  46b18f:	74 3c                	je     46b1cd <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3cd>
  46b191:	44 3b 54 24 ec       	cmp    -0x14(%rsp),%r10d
  46b196:	73 35                	jae    46b1cd <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3cd>
  46b198:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46b19d:	48 8b 5c 24 f0       	mov    -0x10(%rsp),%rbx
  46b1a2:	45 8b 34 98          	mov    (%r8,%rbx,4),%r14d
  46b1a6:	44 89 fb             	mov    %r15d,%ebx
  46b1a9:	4c 01 f3             	add    %r14,%rbx
  46b1ac:	8b 5c 9f 04          	mov    0x4(%rdi,%rbx,4),%ebx
  46b1b0:	48 c1 e3 08          	shl    $0x8,%rbx
  46b1b4:	0f 18 0c 1e          	prefetcht0 (%rsi,%rbx,1)
  46b1b8:	0f 18 4c 1e 40       	prefetcht0 0x40(%rsi,%rbx,1)
  46b1bd:	0f 18 8c 1e 80 00 00 	prefetcht0 0x80(%rsi,%rbx,1)
  46b1c4:	00 
  46b1c5:	0f 18 8c 1e c0 00 00 	prefetcht0 0xc0(%rsi,%rbx,1)
  46b1cc:	00 
  46b1cd:	45 8d 75 01          	lea    0x1(%r13),%r14d
  46b1d1:	a8 02                	test   $0x2,%al
  46b1d3:	74 42                	je     46b217 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x417>
  46b1d5:	41 39 ee             	cmp    %ebp,%r14d
  46b1d8:	73 3d                	jae    46b217 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x417>
  46b1da:	89 e9                	mov    %ebp,%ecx
  46b1dc:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46b1e1:	48 8b 5c 24 f8       	mov    -0x8(%rsp),%rbx
  46b1e6:	41 8b 2c 98          	mov    (%r8,%rbx,4),%ebp
  46b1ea:	44 89 eb             	mov    %r13d,%ebx
  46b1ed:	48 01 eb             	add    %rbp,%rbx
  46b1f0:	89 cd                	mov    %ecx,%ebp
  46b1f2:	8b 4c 24 e8          	mov    -0x18(%rsp),%ecx
  46b1f6:	8b 5c 9f 04          	mov    0x4(%rdi,%rbx,4),%ebx
  46b1fa:	48 c1 e3 08          	shl    $0x8,%rbx
  46b1fe:	0f 18 0c 1e          	prefetcht0 (%rsi,%rbx,1)
  46b202:	0f 18 4c 1e 40       	prefetcht0 0x40(%rsi,%rbx,1)
  46b207:	0f 18 8c 1e 80 00 00 	prefetcht0 0x80(%rsi,%rbx,1)
  46b20e:	00 
  46b20f:	0f 18 8c 1e c0 00 00 	prefetcht0 0xc0(%rsi,%rbx,1)
  46b216:	00 
  46b217:	83 f8 01             	cmp    $0x1,%eax
  46b21a:	0f 84 f0 00 00 00    	je     46b310 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x510>
  46b220:	83 f8 03             	cmp    $0x3,%eax
  46b223:	0f 85 a7 01 00 00    	jne    46b3d0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5d0>
  46b229:	45 89 e0             	mov    %r12d,%r8d
  46b22c:	49 c1 e0 08          	shl    $0x8,%r8
  46b230:	62 a2 7d 48 33 44 06 	vpmovzxwd 0xa0(%rsi,%r8,1),%zmm16
  46b237:	05 
  46b238:	62 a2 7d 48 33 4c 06 	vpmovzxwd 0x80(%rsi,%r8,1),%zmm17
  46b23f:	04 
  46b240:	62 a2 7d 48 33 54 06 	vpmovzxwd 0x60(%rsi,%r8,1),%zmm18
  46b247:	03 
  46b248:	62 a2 7d 48 33 5c 06 	vpmovzxwd 0x40(%rsi,%r8,1),%zmm19
  46b24f:	02 
  46b250:	62 a2 7d 48 33 64 06 	vpmovzxwd 0x20(%rsi,%r8,1),%zmm20
  46b257:	01 
  46b258:	62 a2 7d 48 33 2c 06 	vpmovzxwd (%rsi,%r8,1),%zmm21
  46b25f:	62 a2 7d 48 33 74 06 	vpmovzxwd 0xc0(%rsi,%r8,1),%zmm22
  46b266:	06 
  46b267:	62 a2 7d 48 33 7c 06 	vpmovzxwd 0xe0(%rsi,%r8,1),%zmm23
  46b26e:	07 
  46b26f:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  46b276:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  46b27d:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  46b284:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  46b28b:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  46b292:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  46b299:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  46b2a0:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  46b2a7:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  46b2ad:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  46b2b3:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  46b2b9:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  46b2bf:	62 b1 44 48 58 fc    	vaddps %zmm20,%zmm7,%zmm7
  46b2c5:	62 31 3c 48 58 c5    	vaddps %zmm21,%zmm8,%zmm8
  46b2cb:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  46b2d1:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  46b2d7:	62 31 2c 48 58 d0    	vaddps %zmm16,%zmm10,%zmm10
  46b2dd:	62 31 24 48 58 d9    	vaddps %zmm17,%zmm11,%zmm11
  46b2e3:	62 31 1c 48 58 e2    	vaddps %zmm18,%zmm12,%zmm12
  46b2e9:	62 31 0c 48 58 f3    	vaddps %zmm19,%zmm14,%zmm14
  46b2ef:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  46b2f5:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  46b2fb:	62 31 34 48 58 ce    	vaddps %zmm22,%zmm9,%zmm9
  46b301:	62 b1 54 48 58 ef    	vaddps %zmm23,%zmm5,%zmm5
  46b307:	e9 84 01 00 00       	jmpq   46b490 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x690>
  46b30c:	0f 1f 40 00          	nopl   0x0(%rax)
  46b310:	45 89 e0             	mov    %r12d,%r8d
  46b313:	49 c1 e0 08          	shl    $0x8,%r8
  46b317:	62 a2 7d 48 33 44 06 	vpmovzxwd 0xa0(%rsi,%r8,1),%zmm16
  46b31e:	05 
  46b31f:	62 a2 7d 48 33 4c 06 	vpmovzxwd 0x80(%rsi,%r8,1),%zmm17
  46b326:	04 
  46b327:	62 a2 7d 48 33 54 06 	vpmovzxwd 0x60(%rsi,%r8,1),%zmm18
  46b32e:	03 
  46b32f:	62 a2 7d 48 33 5c 06 	vpmovzxwd 0x40(%rsi,%r8,1),%zmm19
  46b336:	02 
  46b337:	62 a2 7d 48 33 64 06 	vpmovzxwd 0x20(%rsi,%r8,1),%zmm20
  46b33e:	01 
  46b33f:	62 a2 7d 48 33 2c 06 	vpmovzxwd (%rsi,%r8,1),%zmm21
  46b346:	62 a2 7d 48 33 74 06 	vpmovzxwd 0xc0(%rsi,%r8,1),%zmm22
  46b34d:	06 
  46b34e:	62 a2 7d 48 33 7c 06 	vpmovzxwd 0xe0(%rsi,%r8,1),%zmm23
  46b355:	07 
  46b356:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  46b35d:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  46b364:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  46b36b:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  46b372:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  46b379:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  46b380:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  46b387:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  46b38e:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  46b394:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  46b39a:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  46b3a0:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  46b3a6:	62 b1 44 48 58 fc    	vaddps %zmm20,%zmm7,%zmm7
  46b3ac:	62 31 3c 48 58 c5    	vaddps %zmm21,%zmm8,%zmm8
  46b3b2:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  46b3b8:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  46b3be:	e9 cd 00 00 00       	jmpq   46b490 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x690>
  46b3c3:	66 66 66 66 2e 0f 1f 	data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  46b3ca:	84 00 00 00 00 00 
  46b3d0:	44 89 db             	mov    %r11d,%ebx
  46b3d3:	48 c1 e3 08          	shl    $0x8,%rbx
  46b3d7:	62 e2 7d 48 33 44 1e 	vpmovzxwd 0xa0(%rsi,%rbx,1),%zmm16
  46b3de:	05 
  46b3df:	62 e2 7d 48 33 4c 1e 	vpmovzxwd 0x80(%rsi,%rbx,1),%zmm17
  46b3e6:	04 
  46b3e7:	62 e2 7d 48 33 54 1e 	vpmovzxwd 0x60(%rsi,%rbx,1),%zmm18
  46b3ee:	03 
  46b3ef:	62 e2 7d 48 33 5c 1e 	vpmovzxwd 0x40(%rsi,%rbx,1),%zmm19
  46b3f6:	02 
  46b3f7:	62 e2 7d 48 33 64 1e 	vpmovzxwd 0x20(%rsi,%rbx,1),%zmm20
  46b3fe:	01 
  46b3ff:	62 e2 7d 48 33 2c 1e 	vpmovzxwd (%rsi,%rbx,1),%zmm21
  46b406:	62 e2 7d 48 33 74 1e 	vpmovzxwd 0xc0(%rsi,%rbx,1),%zmm22
  46b40d:	06 
  46b40e:	62 e2 7d 48 33 7c 1e 	vpmovzxwd 0xe0(%rsi,%rbx,1),%zmm23
  46b415:	07 
  46b416:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  46b41d:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  46b424:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  46b42b:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  46b432:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  46b439:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  46b440:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  46b447:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  46b44e:	62 31 2c 48 58 d0    	vaddps %zmm16,%zmm10,%zmm10
  46b454:	62 31 24 48 58 d9    	vaddps %zmm17,%zmm11,%zmm11
  46b45a:	62 31 1c 48 58 e2    	vaddps %zmm18,%zmm12,%zmm12
  46b460:	62 31 0c 48 58 f3    	vaddps %zmm19,%zmm14,%zmm14
  46b466:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  46b46c:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  46b472:	62 31 34 48 58 ce    	vaddps %zmm22,%zmm9,%zmm9
  46b478:	62 b1 54 48 58 ef    	vaddps %zmm23,%zmm5,%zmm5
  46b47e:	a8 01                	test   $0x1,%al
  46b480:	74 2e                	je     46b4b0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6b0>
  46b482:	66 66 66 66 66 2e 0f 	data16 data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  46b489:	1f 84 00 00 00 00 00 
  46b490:	41 39 d2             	cmp    %edx,%r10d
  46b493:	73 18                	jae    46b4ad <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6ad>
  46b495:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46b49a:	48 8b 5c 24 f0       	mov    -0x10(%rsp),%rbx
  46b49f:	45 8b 04 98          	mov    (%r8,%rbx,4),%r8d
  46b4a3:	45 89 d4             	mov    %r10d,%r12d
  46b4a6:	4d 01 c4             	add    %r8,%r12
  46b4a9:	46 8b 24 a7          	mov    (%rdi,%r12,4),%r12d
  46b4ad:	45 89 d7             	mov    %r10d,%r15d
  46b4b0:	a8 02                	test   $0x2,%al
  46b4b2:	74 20                	je     46b4d4 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6d4>
  46b4b4:	41 39 ce             	cmp    %ecx,%r14d
  46b4b7:	73 18                	jae    46b4d1 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6d1>
  46b4b9:	48 8b 44 24 d8       	mov    -0x28(%rsp),%rax
  46b4be:	4c 8b 44 24 f8       	mov    -0x8(%rsp),%r8
  46b4c3:	42 8b 04 80          	mov    (%rax,%r8,4),%eax
  46b4c7:	45 89 f2             	mov    %r14d,%r10d
  46b4ca:	49 01 c2             	add    %rax,%r10
  46b4cd:	46 8b 1c 97          	mov    (%rdi,%r10,4),%r11d
  46b4d1:	45 89 f5             	mov    %r14d,%r13d
  46b4d4:	45 85 c9             	test   %r9d,%r9d
  46b4d7:	0f 99 c0             	setns  %al
  46b4da:	41 39 cd             	cmp    %ecx,%r13d
  46b4dd:	41 0f 92 c2          	setb   %r10b
  46b4e1:	41 20 c2             	and    %al,%r10b
  46b4e4:	41 39 d7             	cmp    %edx,%r15d
  46b4e7:	0f 92 c0             	setb   %al
  46b4ea:	0f 82 60 fc ff ff    	jb     46b150 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x350>
  46b4f0:	45 84 d2             	test   %r10b,%r10b
  46b4f3:	0f 85 57 fc ff ff    	jne    46b150 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x350>
  46b4f9:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  46b4fe:	48 8b 54 24 70       	mov    0x70(%rsp),%rdx
  46b503:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  46b508:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
  46b50d:	80 7c 24 d3 00       	cmpb   $0x0,-0x2d(%rsp)
  46b512:	8b 4c 24 e4          	mov    -0x1c(%rsp),%ecx
  46b516:	8b 7c 24 40          	mov    0x40(%rsp),%edi
  46b51a:	0f 84 b8 01 00 00    	je     46b6d8 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8d8>
  46b520:	80 7a 2a 00          	cmpb   $0x0,0x2a(%rdx)
  46b524:	0f 84 7d 03 00 00    	je     46b8a7 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xaa7>
  46b52a:	39 7c 24 ec          	cmp    %edi,-0x14(%rsp)
  46b52e:	0f 87 73 03 00 00    	ja     46b8a7 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xaa7>
  46b534:	62 d1 7d 40 72 d0 10 	vpsrld $0x10,%zmm8,%zmm16
  46b53b:	c1 e1 07             	shl    $0x7,%ecx
  46b53e:	48 63 f9             	movslq %ecx,%rdi
  46b541:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b545:	62 e2 7e 48 33 04 7e 	vpmovdw %zmm16,(%rsi,%rdi,2)
  46b54c:	74 1e                	je     46b56c <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x76c>
  46b54e:	62 e1 3c 58 54 05 9c 	vandps 0x3ed9c(%rip){1to16},%zmm8,%zmm16        # 4aa2f4 <__dso_handle+0x2c>
  46b555:	ed 03 00 
  46b558:	62 31 3c 48 5c c0    	vsubps %zmm16,%zmm8,%zmm8
  46b55e:	62 d1 3d 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm8
  46b565:	62 72 7e 48 33 04 78 	vpmovdw %zmm8,(%rax,%rdi,2)
  46b56c:	62 f1 3d 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm8
  46b573:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b577:	62 72 7e 48 33 44 7e 	vpmovdw %zmm8,0x20(%rsi,%rdi,2)
  46b57e:	01 
  46b57f:	74 1f                	je     46b5a0 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x7a0>
  46b581:	62 71 44 58 54 05 69 	vandps 0x3ed69(%rip){1to16},%zmm7,%zmm8        # 4aa2f4 <__dso_handle+0x2c>
  46b588:	ed 03 00 
  46b58b:	62 d1 44 48 5c f8    	vsubps %zmm8,%zmm7,%zmm7
  46b591:	62 f1 45 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm7
  46b598:	62 f2 7e 48 33 7c 78 	vpmovdw %zmm7,0x20(%rax,%rdi,2)
  46b59f:	01 
  46b5a0:	62 f1 45 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm7
  46b5a7:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b5ab:	62 f2 7e 48 33 7c 7e 	vpmovdw %zmm7,0x40(%rsi,%rdi,2)
  46b5b2:	02 
  46b5b3:	74 1f                	je     46b5d4 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x7d4>
  46b5b5:	62 f1 4c 58 54 3d 35 	vandps 0x3ed35(%rip){1to16},%zmm6,%zmm7        # 4aa2f4 <__dso_handle+0x2c>
  46b5bc:	ed 03 00 
  46b5bf:	62 f1 4c 48 5c f7    	vsubps %zmm7,%zmm6,%zmm6
  46b5c5:	62 f1 4d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm6
  46b5cc:	62 f2 7e 48 33 74 78 	vpmovdw %zmm6,0x40(%rax,%rdi,2)
  46b5d3:	02 
  46b5d4:	62 f1 4d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm6
  46b5db:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b5df:	62 f2 7e 48 33 74 7e 	vpmovdw %zmm6,0x60(%rsi,%rdi,2)
  46b5e6:	03 
  46b5e7:	74 1f                	je     46b608 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x808>
  46b5e9:	62 f1 5c 58 54 35 01 	vandps 0x3ed01(%rip){1to16},%zmm4,%zmm6        # 4aa2f4 <__dso_handle+0x2c>
  46b5f0:	ed 03 00 
  46b5f3:	62 f1 5c 48 5c e6    	vsubps %zmm6,%zmm4,%zmm4
  46b5f9:	62 f1 5d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm4
  46b600:	62 f2 7e 48 33 64 78 	vpmovdw %zmm4,0x60(%rax,%rdi,2)
  46b607:	03 
  46b608:	62 f1 5d 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm4
  46b60f:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b613:	62 f2 7e 48 33 64 7e 	vpmovdw %zmm4,0x80(%rsi,%rdi,2)
  46b61a:	04 
  46b61b:	74 1f                	je     46b63c <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x83c>
  46b61d:	62 f1 64 58 54 25 cd 	vandps 0x3eccd(%rip){1to16},%zmm3,%zmm4        # 4aa2f4 <__dso_handle+0x2c>
  46b624:	ec 03 00 
  46b627:	62 f1 64 48 5c dc    	vsubps %zmm4,%zmm3,%zmm3
  46b62d:	62 f1 65 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm3
  46b634:	62 f2 7e 48 33 5c 78 	vpmovdw %zmm3,0x80(%rax,%rdi,2)
  46b63b:	04 
  46b63c:	62 f1 65 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm3
  46b643:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b647:	62 f2 7e 48 33 5c 7e 	vpmovdw %zmm3,0xa0(%rsi,%rdi,2)
  46b64e:	05 
  46b64f:	74 1f                	je     46b670 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x870>
  46b651:	62 f1 6c 58 54 1d 99 	vandps 0x3ec99(%rip){1to16},%zmm2,%zmm3        # 4aa2f4 <__dso_handle+0x2c>
  46b658:	ec 03 00 
  46b65b:	62 f1 6c 48 5c d3    	vsubps %zmm3,%zmm2,%zmm2
  46b661:	62 f1 6d 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm2
  46b668:	62 f2 7e 48 33 54 78 	vpmovdw %zmm2,0xa0(%rax,%rdi,2)
  46b66f:	05 
  46b670:	62 f1 6d 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm2
  46b677:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b67b:	62 f2 7e 48 33 54 7e 	vpmovdw %zmm2,0xc0(%rsi,%rdi,2)
  46b682:	06 
  46b683:	74 1f                	je     46b6a4 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8a4>
  46b685:	62 f1 74 58 54 15 65 	vandps 0x3ec65(%rip){1to16},%zmm1,%zmm2        # 4aa2f4 <__dso_handle+0x2c>
  46b68c:	ec 03 00 
  46b68f:	62 f1 74 48 5c ca    	vsubps %zmm2,%zmm1,%zmm1
  46b695:	62 f1 75 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm1
  46b69c:	62 f2 7e 48 33 4c 78 	vpmovdw %zmm1,0xc0(%rax,%rdi,2)
  46b6a3:	06 
  46b6a4:	62 f1 75 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm1
  46b6ab:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b6af:	62 f2 7e 48 33 4c 7e 	vpmovdw %zmm1,0xe0(%rsi,%rdi,2)
  46b6b6:	07 
  46b6b7:	74 1f                	je     46b6d8 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8d8>
  46b6b9:	62 f1 7c 58 54 0d 31 	vandps 0x3ec31(%rip){1to16},%zmm0,%zmm1        # 4aa2f4 <__dso_handle+0x2c>
  46b6c0:	ec 03 00 
  46b6c3:	62 f1 7c 48 5c c1    	vsubps %zmm1,%zmm0,%zmm0
  46b6c9:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b6d0:	62 f2 7e 48 33 44 78 	vpmovdw %zmm0,0xe0(%rax,%rdi,2)
  46b6d7:	07 
  46b6d8:	80 7c 24 d4 00       	cmpb   $0x0,-0x2c(%rsp)
  46b6dd:	8b 4c 24 e8          	mov    -0x18(%rsp),%ecx
  46b6e1:	0f 84 5c 02 00 00    	je     46b943 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  46b6e7:	80 7a 2a 00          	cmpb   $0x0,0x2a(%rdx)
  46b6eb:	0f 84 0c 02 00 00    	je     46b8fd <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xafd>
  46b6f1:	39 e9                	cmp    %ebp,%ecx
  46b6f3:	0f 85 04 02 00 00    	jne    46b8fd <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xafd>
  46b6f9:	62 d1 7d 48 72 d5 10 	vpsrld $0x10,%zmm13,%zmm0
  46b700:	41 c1 e1 07          	shl    $0x7,%r9d
  46b704:	49 63 c9             	movslq %r9d,%rcx
  46b707:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b70b:	62 f2 7e 48 33 04 4e 	vpmovdw %zmm0,(%rsi,%rcx,2)
  46b712:	74 1e                	je     46b732 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x932>
  46b714:	62 f1 14 58 54 05 d6 	vandps 0x3ebd6(%rip){1to16},%zmm13,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46b71b:	eb 03 00 
  46b71e:	62 f1 14 48 5c c0    	vsubps %zmm0,%zmm13,%zmm0
  46b724:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b72b:	62 f2 7e 48 33 04 48 	vpmovdw %zmm0,(%rax,%rcx,2)
  46b732:	62 d1 7d 48 72 d7 10 	vpsrld $0x10,%zmm15,%zmm0
  46b739:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b73d:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x20(%rsi,%rcx,2)
  46b744:	01 
  46b745:	74 1f                	je     46b766 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x966>
  46b747:	62 f1 04 58 54 05 a3 	vandps 0x3eba3(%rip){1to16},%zmm15,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46b74e:	eb 03 00 
  46b751:	62 f1 04 48 5c c0    	vsubps %zmm0,%zmm15,%zmm0
  46b757:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b75e:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x20(%rax,%rcx,2)
  46b765:	01 
  46b766:	62 d1 7d 48 72 d6 10 	vpsrld $0x10,%zmm14,%zmm0
  46b76d:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b771:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x40(%rsi,%rcx,2)
  46b778:	02 
  46b779:	74 1f                	je     46b79a <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x99a>
  46b77b:	62 f1 0c 58 54 05 6f 	vandps 0x3eb6f(%rip){1to16},%zmm14,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46b782:	eb 03 00 
  46b785:	62 f1 0c 48 5c c0    	vsubps %zmm0,%zmm14,%zmm0
  46b78b:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b792:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x40(%rax,%rcx,2)
  46b799:	02 
  46b79a:	62 d1 7d 48 72 d4 10 	vpsrld $0x10,%zmm12,%zmm0
  46b7a1:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b7a5:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x60(%rsi,%rcx,2)
  46b7ac:	03 
  46b7ad:	74 1f                	je     46b7ce <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x9ce>
  46b7af:	62 f1 1c 58 54 05 3b 	vandps 0x3eb3b(%rip){1to16},%zmm12,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46b7b6:	eb 03 00 
  46b7b9:	62 f1 1c 48 5c c0    	vsubps %zmm0,%zmm12,%zmm0
  46b7bf:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b7c6:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x60(%rax,%rcx,2)
  46b7cd:	03 
  46b7ce:	62 d1 7d 48 72 d3 10 	vpsrld $0x10,%zmm11,%zmm0
  46b7d5:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b7d9:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x80(%rsi,%rcx,2)
  46b7e0:	04 
  46b7e1:	74 1f                	je     46b802 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa02>
  46b7e3:	62 f1 24 58 54 05 07 	vandps 0x3eb07(%rip){1to16},%zmm11,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46b7ea:	eb 03 00 
  46b7ed:	62 f1 24 48 5c c0    	vsubps %zmm0,%zmm11,%zmm0
  46b7f3:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b7fa:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x80(%rax,%rcx,2)
  46b801:	04 
  46b802:	62 d1 7d 48 72 d2 10 	vpsrld $0x10,%zmm10,%zmm0
  46b809:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b80d:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xa0(%rsi,%rcx,2)
  46b814:	05 
  46b815:	74 1f                	je     46b836 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa36>
  46b817:	62 f1 2c 58 54 05 d3 	vandps 0x3ead3(%rip){1to16},%zmm10,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46b81e:	ea 03 00 
  46b821:	62 f1 2c 48 5c c0    	vsubps %zmm0,%zmm10,%zmm0
  46b827:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b82e:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xa0(%rax,%rcx,2)
  46b835:	05 
  46b836:	62 d1 7d 48 72 d1 10 	vpsrld $0x10,%zmm9,%zmm0
  46b83d:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b841:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xc0(%rsi,%rcx,2)
  46b848:	06 
  46b849:	74 1f                	je     46b86a <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa6a>
  46b84b:	62 f1 34 58 54 05 9f 	vandps 0x3ea9f(%rip){1to16},%zmm9,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46b852:	ea 03 00 
  46b855:	62 f1 34 48 5c c0    	vsubps %zmm0,%zmm9,%zmm0
  46b85b:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b862:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xc0(%rax,%rcx,2)
  46b869:	06 
  46b86a:	62 f1 7d 48 72 d5 10 	vpsrld $0x10,%zmm5,%zmm0
  46b871:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46b875:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xe0(%rsi,%rcx,2)
  46b87c:	07 
  46b87d:	0f 84 c0 00 00 00    	je     46b943 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  46b883:	62 f1 54 58 54 05 67 	vandps 0x3ea67(%rip){1to16},%zmm5,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46b88a:	ea 03 00 
  46b88d:	62 f1 54 48 5c c0    	vsubps %zmm0,%zmm5,%zmm0
  46b893:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46b89a:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xe0(%rax,%rcx,2)
  46b8a1:	07 
  46b8a2:	e9 9c 00 00 00       	jmpq   46b943 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  46b8a7:	c1 e1 07             	shl    $0x7,%ecx
  46b8aa:	48 63 f9             	movslq %ecx,%rdi
  46b8ad:	62 f1 7c 48 11 74 bb 	vmovups %zmm6,0x80(%rbx,%rdi,4)
  46b8b4:	02 
  46b8b5:	62 f1 7c 48 11 7c bb 	vmovups %zmm7,0x40(%rbx,%rdi,4)
  46b8bc:	01 
  46b8bd:	62 71 7c 48 11 04 bb 	vmovups %zmm8,(%rbx,%rdi,4)
  46b8c4:	62 f1 7c 48 11 64 bb 	vmovups %zmm4,0xc0(%rbx,%rdi,4)
  46b8cb:	03 
  46b8cc:	62 f1 7c 48 11 5c bb 	vmovups %zmm3,0x100(%rbx,%rdi,4)
  46b8d3:	04 
  46b8d4:	62 f1 7c 48 11 54 bb 	vmovups %zmm2,0x140(%rbx,%rdi,4)
  46b8db:	05 
  46b8dc:	62 f1 7c 48 11 4c bb 	vmovups %zmm1,0x180(%rbx,%rdi,4)
  46b8e3:	06 
  46b8e4:	62 f1 7c 48 11 44 bb 	vmovups %zmm0,0x1c0(%rbx,%rdi,4)
  46b8eb:	07 
  46b8ec:	80 7c 24 d4 00       	cmpb   $0x0,-0x2c(%rsp)
  46b8f1:	8b 4c 24 e8          	mov    -0x18(%rsp),%ecx
  46b8f5:	0f 85 ec fd ff ff    	jne    46b6e7 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8e7>
  46b8fb:	eb 46                	jmp    46b943 <void gcn_extra_pair_plan::dynamic_pair_window<false, true>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xb43>
  46b8fd:	41 c1 e1 07          	shl    $0x7,%r9d
  46b901:	49 63 c1             	movslq %r9d,%rax
  46b904:	62 71 7c 48 11 74 83 	vmovups %zmm14,0x80(%rbx,%rax,4)
  46b90b:	02 
  46b90c:	62 71 7c 48 11 7c 83 	vmovups %zmm15,0x40(%rbx,%rax,4)
  46b913:	01 
  46b914:	62 71 7c 48 11 2c 83 	vmovups %zmm13,(%rbx,%rax,4)
  46b91b:	62 71 7c 48 11 64 83 	vmovups %zmm12,0xc0(%rbx,%rax,4)
  46b922:	03 
  46b923:	62 71 7c 48 11 5c 83 	vmovups %zmm11,0x100(%rbx,%rax,4)
  46b92a:	04 
  46b92b:	62 71 7c 48 11 54 83 	vmovups %zmm10,0x140(%rbx,%rax,4)
  46b932:	05 
  46b933:	62 71 7c 48 11 4c 83 	vmovups %zmm9,0x180(%rbx,%rax,4)
  46b93a:	06 
  46b93b:	62 f1 7c 48 11 6c 83 	vmovups %zmm5,0x1c0(%rbx,%rax,4)
  46b942:	07 
  46b943:	5b                   	pop    %rbx
  46b944:	41 5c                	pop    %r12
  46b946:	41 5d                	pop    %r13
  46b948:	41 5e                	pop    %r14
  46b94a:	41 5f                	pop    %r15
  46b94c:	5d                   	pop    %rbp
  46b94d:	c5 f8 77             	vzeroupper 
  46b950:	c3                   	retq   
  46b951:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
  46b958:	00 00 00 
  46b95b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

