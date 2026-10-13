000000000046a370 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)>:
  46a370:	55                   	push   %rbp
  46a371:	41 57                	push   %r15
  46a373:	41 56                	push   %r14
  46a375:	41 55                	push   %r13
  46a377:	41 54                	push   %r12
  46a379:	53                   	push   %rbx
  46a37a:	48 89 54 24 d8       	mov    %rdx,-0x28(%rsp)
  46a37f:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  46a384:	0f b6 44 24 50       	movzbl 0x50(%rsp),%eax
  46a389:	8b 6c 24 48          	mov    0x48(%rsp),%ebp
  46a38d:	8b 54 24 40          	mov    0x40(%rsp),%edx
  46a391:	44 8b 74 24 38       	mov    0x38(%rsp),%r14d
  46a396:	4d 63 d0             	movslq %r8d,%r10
  46a399:	4c 89 54 24 f0       	mov    %r10,-0x10(%rsp)
  46a39e:	46 8b 14 91          	mov    (%rcx,%r10,4),%r10d
  46a3a2:	41 39 ea             	cmp    %ebp,%r10d
  46a3a5:	44 0f 43 d5          	cmovae %ebp,%r10d
  46a3a9:	41 39 d2             	cmp    %edx,%r10d
  46a3ac:	41 89 d7             	mov    %edx,%r15d
  46a3af:	44 89 54 24 ec       	mov    %r10d,-0x14(%rsp)
  46a3b4:	45 0f 42 fa          	cmovb  %r10d,%r15d
  46a3b8:	45 39 f7             	cmp    %r14d,%r15d
  46a3bb:	41 0f 97 c5          	seta   %r13b
  46a3bf:	41 89 c3             	mov    %eax,%r11d
  46a3c2:	45 08 eb             	or     %r13b,%r11b
  46a3c5:	45 85 c9             	test   %r9d,%r9d
  46a3c8:	78 78                	js     46a442 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xd2>
  46a3ca:	45 89 cc             	mov    %r9d,%r12d
  46a3cd:	42 8b 0c a1          	mov    (%rcx,%r12,4),%ecx
  46a3d1:	39 e9                	cmp    %ebp,%ecx
  46a3d3:	0f 42 e9             	cmovb  %ecx,%ebp
  46a3d6:	39 d5                	cmp    %edx,%ebp
  46a3d8:	0f 42 d5             	cmovb  %ebp,%edx
  46a3db:	44 39 f2             	cmp    %r14d,%edx
  46a3de:	0f 97 c1             	seta   %cl
  46a3e1:	08 c1                	or     %al,%cl
  46a3e3:	45 84 db             	test   %r11b,%r11b
  46a3e6:	0f 84 ac 00 00 00    	je     46a498 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x128>
  46a3ec:	84 c0                	test   %al,%al
  46a3ee:	0f 84 b0 00 00 00    	je     46a4a4 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x134>
  46a3f4:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  46a3f9:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  46a3fd:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  46a401:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  46a405:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  46a409:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  46a40d:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  46a411:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  46a415:	84 c9                	test   %cl,%cl
  46a417:	0f 85 d8 00 00 00    	jne    46a4f5 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x185>
  46a41d:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  46a422:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  46a429:	00 
  46a42a:	44 89 e9             	mov    %r13d,%ecx
  46a42d:	31 c0                	xor    %eax,%eax
  46a42f:	41 bb 00 00 00 00    	mov    $0x0,%r11d
  46a435:	84 c9                	test   %cl,%cl
  46a437:	0f 85 54 01 00 00    	jne    46a591 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x221>
  46a43d:	e9 6b 01 00 00       	jmpq   46a5ad <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x23d>
  46a442:	45 84 db             	test   %r11b,%r11b
  46a445:	0f 84 9c 09 00 00    	je     46ade7 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  46a44b:	84 c0                	test   %al,%al
  46a44d:	0f 84 a5 01 00 00    	je     46a5f8 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x288>
  46a453:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  46a458:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  46a45c:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  46a460:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  46a464:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  46a468:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  46a46c:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  46a470:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  46a474:	45 39 f7             	cmp    %r14d,%r15d
  46a477:	0f 87 cb 01 00 00    	ja     46a648 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x2d8>
  46a47d:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  46a482:	31 ed                	xor    %ebp,%ebp
  46a484:	45 31 db             	xor    %r11d,%r11d
  46a487:	31 d2                	xor    %edx,%edx
  46a489:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  46a490:	00 
  46a491:	31 c0                	xor    %eax,%eax
  46a493:	e9 30 01 00 00       	jmpq   46a5c8 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x258>
  46a498:	84 c9                	test   %cl,%cl
  46a49a:	0f 84 47 09 00 00    	je     46ade7 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  46a4a0:	31 c9                	xor    %ecx,%ecx
  46a4a2:	eb 54                	jmp    46a4f8 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x188>
  46a4a4:	45 89 c2             	mov    %r8d,%r10d
  46a4a7:	41 c1 e2 07          	shl    $0x7,%r10d
  46a4ab:	4d 63 d2             	movslq %r10d,%r10
  46a4ae:	62 b1 7c 48 10 44 93 	vmovups 0x1c0(%rbx,%r10,4),%zmm0
  46a4b5:	07 
  46a4b6:	62 b1 7c 48 10 4c 93 	vmovups 0x180(%rbx,%r10,4),%zmm1
  46a4bd:	06 
  46a4be:	62 b1 7c 48 10 54 93 	vmovups 0x140(%rbx,%r10,4),%zmm2
  46a4c5:	05 
  46a4c6:	62 b1 7c 48 10 5c 93 	vmovups 0x100(%rbx,%r10,4),%zmm3
  46a4cd:	04 
  46a4ce:	62 31 7c 48 10 04 93 	vmovups (%rbx,%r10,4),%zmm8
  46a4d5:	62 b1 7c 48 10 7c 93 	vmovups 0x40(%rbx,%r10,4),%zmm7
  46a4dc:	01 
  46a4dd:	62 b1 7c 48 10 74 93 	vmovups 0x80(%rbx,%r10,4),%zmm6
  46a4e4:	02 
  46a4e5:	62 b1 7c 48 10 64 93 	vmovups 0xc0(%rbx,%r10,4),%zmm4
  46a4ec:	03 
  46a4ed:	84 c9                	test   %cl,%cl
  46a4ef:	0f 84 28 ff ff ff    	je     46a41d <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xad>
  46a4f5:	44 89 e9             	mov    %r13d,%ecx
  46a4f8:	41 b2 01             	mov    $0x1,%r10b
  46a4fb:	44 89 54 24 d4       	mov    %r10d,-0x2c(%rsp)
  46a500:	84 c0                	test   %al,%al
  46a502:	44 88 5c 24 d3       	mov    %r11b,-0x2d(%rsp)
  46a507:	74 35                	je     46a53e <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x1ce>
  46a509:	c4 41 10 57 ed       	vxorps %xmm13,%xmm13,%xmm13
  46a50e:	c4 41 00 57 ff       	vxorps %xmm15,%xmm15,%xmm15
  46a513:	c4 41 08 57 f6       	vxorps %xmm14,%xmm14,%xmm14
  46a518:	c4 41 18 57 e4       	vxorps %xmm12,%xmm12,%xmm12
  46a51d:	c4 41 20 57 db       	vxorps %xmm11,%xmm11,%xmm11
  46a522:	c4 41 28 57 d2       	vxorps %xmm10,%xmm10,%xmm10
  46a527:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  46a52c:	c5 d0 57 ed          	vxorps %xmm5,%xmm5,%xmm5
  46a530:	31 c0                	xor    %eax,%eax
  46a532:	41 bb 00 00 00 00    	mov    $0x0,%r11d
  46a538:	84 c9                	test   %cl,%cl
  46a53a:	75 55                	jne    46a591 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x221>
  46a53c:	eb 6f                	jmp    46a5ad <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x23d>
  46a53e:	44 89 c8             	mov    %r9d,%eax
  46a541:	c1 e0 07             	shl    $0x7,%eax
  46a544:	48 98                	cltq   
  46a546:	62 f1 7c 48 10 6c 83 	vmovups 0x1c0(%rbx,%rax,4),%zmm5
  46a54d:	07 
  46a54e:	62 71 7c 48 10 4c 83 	vmovups 0x180(%rbx,%rax,4),%zmm9
  46a555:	06 
  46a556:	62 71 7c 48 10 54 83 	vmovups 0x140(%rbx,%rax,4),%zmm10
  46a55d:	05 
  46a55e:	62 71 7c 48 10 5c 83 	vmovups 0x100(%rbx,%rax,4),%zmm11
  46a565:	04 
  46a566:	62 71 7c 48 10 2c 83 	vmovups (%rbx,%rax,4),%zmm13
  46a56d:	62 71 7c 48 10 7c 83 	vmovups 0x40(%rbx,%rax,4),%zmm15
  46a574:	01 
  46a575:	62 71 7c 48 10 74 83 	vmovups 0x80(%rbx,%rax,4),%zmm14
  46a57c:	02 
  46a57d:	62 71 7c 48 10 64 83 	vmovups 0xc0(%rbx,%rax,4),%zmm12
  46a584:	03 
  46a585:	31 c0                	xor    %eax,%eax
  46a587:	41 bb 00 00 00 00    	mov    $0x0,%r11d
  46a58d:	84 c9                	test   %cl,%cl
  46a58f:	74 1c                	je     46a5ad <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x23d>
  46a591:	48 8b 4c 24 d8       	mov    -0x28(%rsp),%rcx
  46a596:	4c 8b 54 24 f0       	mov    -0x10(%rsp),%r10
  46a59b:	42 8b 0c 91          	mov    (%rcx,%r10,4),%ecx
  46a59f:	45 89 f2             	mov    %r14d,%r10d
  46a5a2:	49 01 ca             	add    %rcx,%r10
  46a5a5:	48 8b 4f 18          	mov    0x18(%rdi),%rcx
  46a5a9:	46 8b 1c 91          	mov    (%rcx,%r10,4),%r11d
  46a5ad:	44 39 f2             	cmp    %r14d,%edx
  46a5b0:	76 16                	jbe    46a5c8 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x258>
  46a5b2:	48 8b 44 24 d8       	mov    -0x28(%rsp),%rax
  46a5b7:	42 8b 04 a0          	mov    (%rax,%r12,4),%eax
  46a5bb:	44 89 f1             	mov    %r14d,%ecx
  46a5be:	48 01 c1             	add    %rax,%rcx
  46a5c1:	48 8b 47 18          	mov    0x18(%rdi),%rax
  46a5c5:	8b 04 88             	mov    (%rax,%rcx,4),%eax
  46a5c8:	45 85 c9             	test   %r9d,%r9d
  46a5cb:	0f 99 c1             	setns  %cl
  46a5ce:	44 39 f2             	cmp    %r14d,%edx
  46a5d1:	41 0f 97 c2          	seta   %r10b
  46a5d5:	41 20 ca             	and    %cl,%r10b
  46a5d8:	45 39 f7             	cmp    %r14d,%r15d
  46a5db:	77 09                	ja     46a5e6 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x276>
  46a5dd:	45 84 d2             	test   %r10b,%r10b
  46a5e0:	0f 84 b4 03 00 00    	je     46a99a <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x62a>
  46a5e6:	44 89 44 24 e8       	mov    %r8d,-0x18(%rsp)
  46a5eb:	89 6c 24 e4          	mov    %ebp,-0x1c(%rsp)
  46a5ef:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  46a5f3:	e9 8c 00 00 00       	jmpq   46a684 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x314>
  46a5f8:	44 89 c0             	mov    %r8d,%eax
  46a5fb:	c1 e0 07             	shl    $0x7,%eax
  46a5fe:	48 98                	cltq   
  46a600:	62 f1 7c 48 10 44 83 	vmovups 0x1c0(%rbx,%rax,4),%zmm0
  46a607:	07 
  46a608:	62 f1 7c 48 10 4c 83 	vmovups 0x180(%rbx,%rax,4),%zmm1
  46a60f:	06 
  46a610:	62 f1 7c 48 10 54 83 	vmovups 0x140(%rbx,%rax,4),%zmm2
  46a617:	05 
  46a618:	62 f1 7c 48 10 5c 83 	vmovups 0x100(%rbx,%rax,4),%zmm3
  46a61f:	04 
  46a620:	62 71 7c 48 10 04 83 	vmovups (%rbx,%rax,4),%zmm8
  46a627:	62 f1 7c 48 10 7c 83 	vmovups 0x40(%rbx,%rax,4),%zmm7
  46a62e:	01 
  46a62f:	62 f1 7c 48 10 74 83 	vmovups 0x80(%rbx,%rax,4),%zmm6
  46a636:	02 
  46a637:	62 f1 7c 48 10 64 83 	vmovups 0xc0(%rbx,%rax,4),%zmm4
  46a63e:	03 
  46a63f:	45 39 f7             	cmp    %r14d,%r15d
  46a642:	0f 86 35 fe ff ff    	jbe    46a47d <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x10d>
  46a648:	44 89 44 24 e8       	mov    %r8d,-0x18(%rsp)
  46a64d:	48 8b 44 24 d8       	mov    -0x28(%rsp),%rax
  46a652:	48 8b 4c 24 f0       	mov    -0x10(%rsp),%rcx
  46a657:	8b 04 88             	mov    (%rax,%rcx,4),%eax
  46a65a:	44 89 f1             	mov    %r14d,%ecx
  46a65d:	48 01 c1             	add    %rax,%rcx
  46a660:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  46a664:	44 8b 1c 8f          	mov    (%rdi,%rcx,4),%r11d
  46a668:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  46a66d:	45 31 d2             	xor    %r10d,%r10d
  46a670:	31 c0                	xor    %eax,%eax
  46a672:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  46a679:	00 
  46a67a:	31 d2                	xor    %edx,%edx
  46a67c:	c7 44 24 e4 00 00 00 	movl   $0x0,-0x1c(%rsp)
  46a683:	00 
  46a684:	49 63 c9             	movslq %r9d,%rcx
  46a687:	48 89 4c 24 f8       	mov    %rcx,-0x8(%rsp)
  46a68c:	45 89 f4             	mov    %r14d,%r12d
  46a68f:	44 89 c9             	mov    %r9d,%ecx
  46a692:	66 66 66 66 66 2e 0f 	data16 data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  46a699:	1f 84 00 00 00 00 00 
  46a6a0:	41 f6 c5 01          	test   $0x1,%r13b
  46a6a4:	74 3a                	je     46a6e0 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x370>
  46a6a6:	44 39 d8             	cmp    %r11d,%eax
  46a6a9:	41 bd 03 00 00 00    	mov    $0x3,%r13d
  46a6af:	41 83 dd 00          	sbb    $0x0,%r13d
  46a6b3:	41 f6 c2 01          	test   $0x1,%r10b
  46a6b7:	41 b8 01 00 00 00    	mov    $0x1,%r8d
  46a6bd:	45 0f 44 e8          	cmove  %r8d,%r13d
  46a6c1:	41 39 c3             	cmp    %eax,%r11d
  46a6c4:	45 0f 42 e8          	cmovb  %r8d,%r13d
  46a6c8:	45 8d 56 01          	lea    0x1(%r14),%r10d
  46a6cc:	41 f6 c5 01          	test   $0x1,%r13b
  46a6d0:	75 23                	jne    46a6f5 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x385>
  46a6d2:	eb 65                	jmp    46a739 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3c9>
  46a6d4:	66 66 66 2e 0f 1f 84 	data16 data16 nopw %cs:0x0(%rax,%rax,1)
  46a6db:	00 00 00 00 00 
  46a6e0:	45 0f b6 ea          	movzbl %r10b,%r13d
  46a6e4:	41 83 e5 01          	and    $0x1,%r13d
  46a6e8:	45 01 ed             	add    %r13d,%r13d
  46a6eb:	45 8d 56 01          	lea    0x1(%r14),%r10d
  46a6ef:	41 f6 c5 01          	test   $0x1,%r13b
  46a6f3:	74 44                	je     46a739 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3c9>
  46a6f5:	44 3b 54 24 ec       	cmp    -0x14(%rsp),%r10d
  46a6fa:	73 3d                	jae    46a739 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3c9>
  46a6fc:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46a701:	4c 8b 4c 24 f0       	mov    -0x10(%rsp),%r9
  46a706:	47 8b 0c 88          	mov    (%r8,%r9,4),%r9d
  46a70a:	44 89 f3             	mov    %r14d,%ebx
  46a70d:	4c 01 cb             	add    %r9,%rbx
  46a710:	44 8b 44 9f 04       	mov    0x4(%rdi,%rbx,4),%r8d
  46a715:	41 89 c9             	mov    %ecx,%r9d
  46a718:	49 c1 e0 08          	shl    $0x8,%r8
  46a71c:	42 0f 18 0c 06       	prefetcht0 (%rsi,%r8,1)
  46a721:	42 0f 18 4c 06 40    	prefetcht0 0x40(%rsi,%r8,1)
  46a727:	42 0f 18 8c 06 80 00 	prefetcht0 0x80(%rsi,%r8,1)
  46a72e:	00 00 
  46a730:	42 0f 18 8c 06 c0 00 	prefetcht0 0xc0(%rsi,%r8,1)
  46a737:	00 00 
  46a739:	41 8d 6c 24 01       	lea    0x1(%r12),%ebp
  46a73e:	41 f6 c5 02          	test   $0x2,%r13b
  46a742:	74 43                	je     46a787 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x417>
  46a744:	3b 6c 24 e4          	cmp    -0x1c(%rsp),%ebp
  46a748:	73 3d                	jae    46a787 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x417>
  46a74a:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46a74f:	4c 8b 4c 24 f8       	mov    -0x8(%rsp),%r9
  46a754:	43 8b 1c 88          	mov    (%r8,%r9,4),%ebx
  46a758:	45 89 e0             	mov    %r12d,%r8d
  46a75b:	49 01 d8             	add    %rbx,%r8
  46a75e:	41 89 c9             	mov    %ecx,%r9d
  46a761:	46 8b 44 87 04       	mov    0x4(%rdi,%r8,4),%r8d
  46a766:	49 c1 e0 08          	shl    $0x8,%r8
  46a76a:	42 0f 18 0c 06       	prefetcht0 (%rsi,%r8,1)
  46a76f:	42 0f 18 4c 06 40    	prefetcht0 0x40(%rsi,%r8,1)
  46a775:	42 0f 18 8c 06 80 00 	prefetcht0 0x80(%rsi,%r8,1)
  46a77c:	00 00 
  46a77e:	42 0f 18 8c 06 c0 00 	prefetcht0 0xc0(%rsi,%r8,1)
  46a785:	00 00 
  46a787:	41 f6 c5 01          	test   $0x1,%r13b
  46a78b:	75 23                	jne    46a7b0 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x440>
  46a78d:	41 f6 c5 02          	test   $0x2,%r13b
  46a791:	0f 85 d1 00 00 00    	jne    46a868 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x4f8>
  46a797:	41 f6 c5 01          	test   $0x1,%r13b
  46a79b:	0f 85 7f 01 00 00    	jne    46a920 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5b0>
  46a7a1:	41 f6 c5 02          	test   $0x2,%r13b
  46a7a5:	0f 85 9b 01 00 00    	jne    46a946 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5d6>
  46a7ab:	e9 b5 01 00 00       	jmpq   46a965 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5f5>
  46a7b0:	45 89 d8             	mov    %r11d,%r8d
  46a7b3:	49 c1 e0 08          	shl    $0x8,%r8
  46a7b7:	62 a2 7d 48 33 44 06 	vpmovzxwd 0xa0(%rsi,%r8,1),%zmm16
  46a7be:	05 
  46a7bf:	62 a2 7d 48 33 4c 06 	vpmovzxwd 0x80(%rsi,%r8,1),%zmm17
  46a7c6:	04 
  46a7c7:	62 a2 7d 48 33 54 06 	vpmovzxwd 0x60(%rsi,%r8,1),%zmm18
  46a7ce:	03 
  46a7cf:	62 a2 7d 48 33 5c 06 	vpmovzxwd 0x40(%rsi,%r8,1),%zmm19
  46a7d6:	02 
  46a7d7:	62 a2 7d 48 33 64 06 	vpmovzxwd 0x20(%rsi,%r8,1),%zmm20
  46a7de:	01 
  46a7df:	62 a2 7d 48 33 2c 06 	vpmovzxwd (%rsi,%r8,1),%zmm21
  46a7e6:	62 a2 7d 48 33 74 06 	vpmovzxwd 0xc0(%rsi,%r8,1),%zmm22
  46a7ed:	06 
  46a7ee:	62 a2 7d 48 33 7c 06 	vpmovzxwd 0xe0(%rsi,%r8,1),%zmm23
  46a7f5:	07 
  46a7f6:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  46a7fd:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  46a804:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  46a80b:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  46a812:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  46a819:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  46a820:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  46a827:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  46a82e:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  46a834:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  46a83a:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  46a840:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  46a846:	62 b1 44 48 58 fc    	vaddps %zmm20,%zmm7,%zmm7
  46a84c:	62 31 3c 48 58 c5    	vaddps %zmm21,%zmm8,%zmm8
  46a852:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  46a858:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  46a85e:	41 f6 c5 02          	test   $0x2,%r13b
  46a862:	0f 84 2f ff ff ff    	je     46a797 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x427>
  46a868:	41 89 c0             	mov    %eax,%r8d
  46a86b:	49 c1 e0 08          	shl    $0x8,%r8
  46a86f:	62 a2 7d 48 33 44 06 	vpmovzxwd 0xa0(%rsi,%r8,1),%zmm16
  46a876:	05 
  46a877:	62 a2 7d 48 33 4c 06 	vpmovzxwd 0x80(%rsi,%r8,1),%zmm17
  46a87e:	04 
  46a87f:	62 a2 7d 48 33 54 06 	vpmovzxwd 0x60(%rsi,%r8,1),%zmm18
  46a886:	03 
  46a887:	62 a2 7d 48 33 5c 06 	vpmovzxwd 0x40(%rsi,%r8,1),%zmm19
  46a88e:	02 
  46a88f:	62 a2 7d 48 33 64 06 	vpmovzxwd 0x20(%rsi,%r8,1),%zmm20
  46a896:	01 
  46a897:	62 a2 7d 48 33 2c 06 	vpmovzxwd (%rsi,%r8,1),%zmm21
  46a89e:	62 a2 7d 48 33 74 06 	vpmovzxwd 0xc0(%rsi,%r8,1),%zmm22
  46a8a5:	06 
  46a8a6:	62 a2 7d 48 33 7c 06 	vpmovzxwd 0xe0(%rsi,%r8,1),%zmm23
  46a8ad:	07 
  46a8ae:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  46a8b5:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  46a8bc:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  46a8c3:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  46a8ca:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  46a8d1:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  46a8d8:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  46a8df:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  46a8e6:	62 31 2c 48 58 d0    	vaddps %zmm16,%zmm10,%zmm10
  46a8ec:	62 31 24 48 58 d9    	vaddps %zmm17,%zmm11,%zmm11
  46a8f2:	62 31 1c 48 58 e2    	vaddps %zmm18,%zmm12,%zmm12
  46a8f8:	62 31 0c 48 58 f3    	vaddps %zmm19,%zmm14,%zmm14
  46a8fe:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  46a904:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  46a90a:	62 31 34 48 58 ce    	vaddps %zmm22,%zmm9,%zmm9
  46a910:	62 b1 54 48 58 ef    	vaddps %zmm23,%zmm5,%zmm5
  46a916:	41 f6 c5 01          	test   $0x1,%r13b
  46a91a:	0f 84 81 fe ff ff    	je     46a7a1 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x431>
  46a920:	45 39 fa             	cmp    %r15d,%r10d
  46a923:	73 18                	jae    46a93d <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5cd>
  46a925:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46a92a:	4c 8b 5c 24 f0       	mov    -0x10(%rsp),%r11
  46a92f:	47 8b 1c 98          	mov    (%r8,%r11,4),%r11d
  46a933:	45 89 d6             	mov    %r10d,%r14d
  46a936:	4d 01 de             	add    %r11,%r14
  46a939:	46 8b 1c b7          	mov    (%rdi,%r14,4),%r11d
  46a93d:	45 89 d6             	mov    %r10d,%r14d
  46a940:	41 f6 c5 02          	test   $0x2,%r13b
  46a944:	74 1f                	je     46a965 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5f5>
  46a946:	39 d5                	cmp    %edx,%ebp
  46a948:	73 18                	jae    46a962 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5f2>
  46a94a:	48 8b 44 24 d8       	mov    -0x28(%rsp),%rax
  46a94f:	4c 8b 44 24 f8       	mov    -0x8(%rsp),%r8
  46a954:	42 8b 04 80          	mov    (%rax,%r8,4),%eax
  46a958:	41 89 ea             	mov    %ebp,%r10d
  46a95b:	49 01 c2             	add    %rax,%r10
  46a95e:	42 8b 04 97          	mov    (%rdi,%r10,4),%eax
  46a962:	41 89 ec             	mov    %ebp,%r12d
  46a965:	45 85 c9             	test   %r9d,%r9d
  46a968:	41 0f 99 c0          	setns  %r8b
  46a96c:	41 39 d4             	cmp    %edx,%r12d
  46a96f:	41 0f 92 c2          	setb   %r10b
  46a973:	45 20 c2             	and    %r8b,%r10b
  46a976:	45 39 fe             	cmp    %r15d,%r14d
  46a979:	41 0f 92 c5          	setb   %r13b
  46a97d:	0f 82 1d fd ff ff    	jb     46a6a0 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x330>
  46a983:	45 84 d2             	test   %r10b,%r10b
  46a986:	0f 85 14 fd ff ff    	jne    46a6a0 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x330>
  46a98c:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  46a991:	8b 6c 24 e4          	mov    -0x1c(%rsp),%ebp
  46a995:	44 8b 44 24 e8       	mov    -0x18(%rsp),%r8d
  46a99a:	4c 8b 54 24 70       	mov    0x70(%rsp),%r10
  46a99f:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  46a9a4:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
  46a9a9:	80 7c 24 d3 00       	cmpb   $0x0,-0x2d(%rsp)
  46a9ae:	0f 84 c6 01 00 00    	je     46ab7a <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x80a>
  46a9b4:	41 80 7a 2a 00       	cmpb   $0x0,0x2a(%r10)
  46a9b9:	0f 84 8f 03 00 00    	je     46ad4e <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x9de>
  46a9bf:	8b 4c 24 ec          	mov    -0x14(%rsp),%ecx
  46a9c3:	3b 4c 24 40          	cmp    0x40(%rsp),%ecx
  46a9c7:	0f 87 81 03 00 00    	ja     46ad4e <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x9de>
  46a9cd:	62 d1 7d 40 72 d0 10 	vpsrld $0x10,%zmm8,%zmm16
  46a9d4:	41 c1 e0 07          	shl    $0x7,%r8d
  46a9d8:	49 63 f8             	movslq %r8d,%rdi
  46a9db:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46a9e0:	62 e2 7e 48 33 04 7e 	vpmovdw %zmm16,(%rsi,%rdi,2)
  46a9e7:	74 1e                	je     46aa07 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x697>
  46a9e9:	62 e1 3c 58 54 05 01 	vandps 0x3f901(%rip){1to16},%zmm8,%zmm16        # 4aa2f4 <__dso_handle+0x2c>
  46a9f0:	f9 03 00 
  46a9f3:	62 31 3c 48 5c c0    	vsubps %zmm16,%zmm8,%zmm8
  46a9f9:	62 d1 3d 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm8
  46aa00:	62 72 7e 48 33 04 78 	vpmovdw %zmm8,(%rax,%rdi,2)
  46aa07:	62 f1 3d 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm8
  46aa0e:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46aa13:	62 72 7e 48 33 44 7e 	vpmovdw %zmm8,0x20(%rsi,%rdi,2)
  46aa1a:	01 
  46aa1b:	74 1f                	je     46aa3c <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6cc>
  46aa1d:	62 71 44 58 54 05 cd 	vandps 0x3f8cd(%rip){1to16},%zmm7,%zmm8        # 4aa2f4 <__dso_handle+0x2c>
  46aa24:	f8 03 00 
  46aa27:	62 d1 44 48 5c f8    	vsubps %zmm8,%zmm7,%zmm7
  46aa2d:	62 f1 45 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm7
  46aa34:	62 f2 7e 48 33 7c 78 	vpmovdw %zmm7,0x20(%rax,%rdi,2)
  46aa3b:	01 
  46aa3c:	62 f1 45 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm7
  46aa43:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46aa48:	62 f2 7e 48 33 7c 7e 	vpmovdw %zmm7,0x40(%rsi,%rdi,2)
  46aa4f:	02 
  46aa50:	74 1f                	je     46aa71 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x701>
  46aa52:	62 f1 4c 58 54 3d 98 	vandps 0x3f898(%rip){1to16},%zmm6,%zmm7        # 4aa2f4 <__dso_handle+0x2c>
  46aa59:	f8 03 00 
  46aa5c:	62 f1 4c 48 5c f7    	vsubps %zmm7,%zmm6,%zmm6
  46aa62:	62 f1 4d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm6
  46aa69:	62 f2 7e 48 33 74 78 	vpmovdw %zmm6,0x40(%rax,%rdi,2)
  46aa70:	02 
  46aa71:	62 f1 4d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm6
  46aa78:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46aa7d:	62 f2 7e 48 33 74 7e 	vpmovdw %zmm6,0x60(%rsi,%rdi,2)
  46aa84:	03 
  46aa85:	74 1f                	je     46aaa6 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x736>
  46aa87:	62 f1 5c 58 54 35 63 	vandps 0x3f863(%rip){1to16},%zmm4,%zmm6        # 4aa2f4 <__dso_handle+0x2c>
  46aa8e:	f8 03 00 
  46aa91:	62 f1 5c 48 5c e6    	vsubps %zmm6,%zmm4,%zmm4
  46aa97:	62 f1 5d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm4
  46aa9e:	62 f2 7e 48 33 64 78 	vpmovdw %zmm4,0x60(%rax,%rdi,2)
  46aaa5:	03 
  46aaa6:	62 f1 5d 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm4
  46aaad:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46aab2:	62 f2 7e 48 33 64 7e 	vpmovdw %zmm4,0x80(%rsi,%rdi,2)
  46aab9:	04 
  46aaba:	74 1f                	je     46aadb <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x76b>
  46aabc:	62 f1 64 58 54 25 2e 	vandps 0x3f82e(%rip){1to16},%zmm3,%zmm4        # 4aa2f4 <__dso_handle+0x2c>
  46aac3:	f8 03 00 
  46aac6:	62 f1 64 48 5c dc    	vsubps %zmm4,%zmm3,%zmm3
  46aacc:	62 f1 65 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm3
  46aad3:	62 f2 7e 48 33 5c 78 	vpmovdw %zmm3,0x80(%rax,%rdi,2)
  46aada:	04 
  46aadb:	62 f1 65 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm3
  46aae2:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46aae7:	62 f2 7e 48 33 5c 7e 	vpmovdw %zmm3,0xa0(%rsi,%rdi,2)
  46aaee:	05 
  46aaef:	74 1f                	je     46ab10 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x7a0>
  46aaf1:	62 f1 6c 58 54 1d f9 	vandps 0x3f7f9(%rip){1to16},%zmm2,%zmm3        # 4aa2f4 <__dso_handle+0x2c>
  46aaf8:	f7 03 00 
  46aafb:	62 f1 6c 48 5c d3    	vsubps %zmm3,%zmm2,%zmm2
  46ab01:	62 f1 6d 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm2
  46ab08:	62 f2 7e 48 33 54 78 	vpmovdw %zmm2,0xa0(%rax,%rdi,2)
  46ab0f:	05 
  46ab10:	62 f1 6d 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm2
  46ab17:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46ab1c:	62 f2 7e 48 33 54 7e 	vpmovdw %zmm2,0xc0(%rsi,%rdi,2)
  46ab23:	06 
  46ab24:	74 1f                	je     46ab45 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x7d5>
  46ab26:	62 f1 74 58 54 15 c4 	vandps 0x3f7c4(%rip){1to16},%zmm1,%zmm2        # 4aa2f4 <__dso_handle+0x2c>
  46ab2d:	f7 03 00 
  46ab30:	62 f1 74 48 5c ca    	vsubps %zmm2,%zmm1,%zmm1
  46ab36:	62 f1 75 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm1
  46ab3d:	62 f2 7e 48 33 4c 78 	vpmovdw %zmm1,0xc0(%rax,%rdi,2)
  46ab44:	06 
  46ab45:	62 f1 75 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm1
  46ab4c:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46ab51:	62 f2 7e 48 33 4c 7e 	vpmovdw %zmm1,0xe0(%rsi,%rdi,2)
  46ab58:	07 
  46ab59:	74 1f                	je     46ab7a <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x80a>
  46ab5b:	62 f1 7c 58 54 0d 8f 	vandps 0x3f78f(%rip){1to16},%zmm0,%zmm1        # 4aa2f4 <__dso_handle+0x2c>
  46ab62:	f7 03 00 
  46ab65:	62 f1 7c 48 5c c1    	vsubps %zmm1,%zmm0,%zmm0
  46ab6b:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46ab72:	62 f2 7e 48 33 44 78 	vpmovdw %zmm0,0xe0(%rax,%rdi,2)
  46ab79:	07 
  46ab7a:	80 7c 24 d4 00       	cmpb   $0x0,-0x2c(%rsp)
  46ab7f:	0f 84 62 02 00 00    	je     46ade7 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  46ab85:	41 80 7a 2a 00       	cmpb   $0x0,0x2a(%r10)
  46ab8a:	0f 84 11 02 00 00    	je     46ada1 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa31>
  46ab90:	39 ea                	cmp    %ebp,%edx
  46ab92:	0f 85 09 02 00 00    	jne    46ada1 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa31>
  46ab98:	62 d1 7d 48 72 d5 10 	vpsrld $0x10,%zmm13,%zmm0
  46ab9f:	41 c1 e1 07          	shl    $0x7,%r9d
  46aba3:	49 63 c9             	movslq %r9d,%rcx
  46aba6:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46abab:	62 f2 7e 48 33 04 4e 	vpmovdw %zmm0,(%rsi,%rcx,2)
  46abb2:	74 1e                	je     46abd2 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x862>
  46abb4:	62 f1 14 58 54 05 36 	vandps 0x3f736(%rip){1to16},%zmm13,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46abbb:	f7 03 00 
  46abbe:	62 f1 14 48 5c c0    	vsubps %zmm0,%zmm13,%zmm0
  46abc4:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46abcb:	62 f2 7e 48 33 04 48 	vpmovdw %zmm0,(%rax,%rcx,2)
  46abd2:	62 d1 7d 48 72 d7 10 	vpsrld $0x10,%zmm15,%zmm0
  46abd9:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46abde:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x20(%rsi,%rcx,2)
  46abe5:	01 
  46abe6:	74 1f                	je     46ac07 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x897>
  46abe8:	62 f1 04 58 54 05 02 	vandps 0x3f702(%rip){1to16},%zmm15,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46abef:	f7 03 00 
  46abf2:	62 f1 04 48 5c c0    	vsubps %zmm0,%zmm15,%zmm0
  46abf8:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46abff:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x20(%rax,%rcx,2)
  46ac06:	01 
  46ac07:	62 d1 7d 48 72 d6 10 	vpsrld $0x10,%zmm14,%zmm0
  46ac0e:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46ac13:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x40(%rsi,%rcx,2)
  46ac1a:	02 
  46ac1b:	74 1f                	je     46ac3c <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8cc>
  46ac1d:	62 f1 0c 58 54 05 cd 	vandps 0x3f6cd(%rip){1to16},%zmm14,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46ac24:	f6 03 00 
  46ac27:	62 f1 0c 48 5c c0    	vsubps %zmm0,%zmm14,%zmm0
  46ac2d:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46ac34:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x40(%rax,%rcx,2)
  46ac3b:	02 
  46ac3c:	62 d1 7d 48 72 d4 10 	vpsrld $0x10,%zmm12,%zmm0
  46ac43:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46ac48:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x60(%rsi,%rcx,2)
  46ac4f:	03 
  46ac50:	74 1f                	je     46ac71 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x901>
  46ac52:	62 f1 1c 58 54 05 98 	vandps 0x3f698(%rip){1to16},%zmm12,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46ac59:	f6 03 00 
  46ac5c:	62 f1 1c 48 5c c0    	vsubps %zmm0,%zmm12,%zmm0
  46ac62:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46ac69:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x60(%rax,%rcx,2)
  46ac70:	03 
  46ac71:	62 d1 7d 48 72 d3 10 	vpsrld $0x10,%zmm11,%zmm0
  46ac78:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46ac7d:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x80(%rsi,%rcx,2)
  46ac84:	04 
  46ac85:	74 1f                	je     46aca6 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x936>
  46ac87:	62 f1 24 58 54 05 63 	vandps 0x3f663(%rip){1to16},%zmm11,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46ac8e:	f6 03 00 
  46ac91:	62 f1 24 48 5c c0    	vsubps %zmm0,%zmm11,%zmm0
  46ac97:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46ac9e:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x80(%rax,%rcx,2)
  46aca5:	04 
  46aca6:	62 d1 7d 48 72 d2 10 	vpsrld $0x10,%zmm10,%zmm0
  46acad:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46acb2:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xa0(%rsi,%rcx,2)
  46acb9:	05 
  46acba:	74 1f                	je     46acdb <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x96b>
  46acbc:	62 f1 2c 58 54 05 2e 	vandps 0x3f62e(%rip){1to16},%zmm10,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46acc3:	f6 03 00 
  46acc6:	62 f1 2c 48 5c c0    	vsubps %zmm0,%zmm10,%zmm0
  46accc:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46acd3:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xa0(%rax,%rcx,2)
  46acda:	05 
  46acdb:	62 d1 7d 48 72 d1 10 	vpsrld $0x10,%zmm9,%zmm0
  46ace2:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46ace7:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xc0(%rsi,%rcx,2)
  46acee:	06 
  46acef:	74 1f                	je     46ad10 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x9a0>
  46acf1:	62 f1 34 58 54 05 f9 	vandps 0x3f5f9(%rip){1to16},%zmm9,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46acf8:	f5 03 00 
  46acfb:	62 f1 34 48 5c c0    	vsubps %zmm0,%zmm9,%zmm0
  46ad01:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46ad08:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xc0(%rax,%rcx,2)
  46ad0f:	06 
  46ad10:	62 f1 7d 48 72 d5 10 	vpsrld $0x10,%zmm5,%zmm0
  46ad17:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46ad1c:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xe0(%rsi,%rcx,2)
  46ad23:	07 
  46ad24:	0f 84 bd 00 00 00    	je     46ade7 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  46ad2a:	62 f1 54 58 54 05 c0 	vandps 0x3f5c0(%rip){1to16},%zmm5,%zmm0        # 4aa2f4 <__dso_handle+0x2c>
  46ad31:	f5 03 00 
  46ad34:	62 f1 54 48 5c c0    	vsubps %zmm0,%zmm5,%zmm0
  46ad3a:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46ad41:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xe0(%rax,%rcx,2)
  46ad48:	07 
  46ad49:	e9 99 00 00 00       	jmpq   46ade7 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  46ad4e:	41 c1 e0 07          	shl    $0x7,%r8d
  46ad52:	49 63 f8             	movslq %r8d,%rdi
  46ad55:	62 f1 7c 48 11 74 bb 	vmovups %zmm6,0x80(%rbx,%rdi,4)
  46ad5c:	02 
  46ad5d:	62 f1 7c 48 11 7c bb 	vmovups %zmm7,0x40(%rbx,%rdi,4)
  46ad64:	01 
  46ad65:	62 71 7c 48 11 04 bb 	vmovups %zmm8,(%rbx,%rdi,4)
  46ad6c:	62 f1 7c 48 11 64 bb 	vmovups %zmm4,0xc0(%rbx,%rdi,4)
  46ad73:	03 
  46ad74:	62 f1 7c 48 11 5c bb 	vmovups %zmm3,0x100(%rbx,%rdi,4)
  46ad7b:	04 
  46ad7c:	62 f1 7c 48 11 54 bb 	vmovups %zmm2,0x140(%rbx,%rdi,4)
  46ad83:	05 
  46ad84:	62 f1 7c 48 11 4c bb 	vmovups %zmm1,0x180(%rbx,%rdi,4)
  46ad8b:	06 
  46ad8c:	62 f1 7c 48 11 44 bb 	vmovups %zmm0,0x1c0(%rbx,%rdi,4)
  46ad93:	07 
  46ad94:	80 7c 24 d4 00       	cmpb   $0x0,-0x2c(%rsp)
  46ad99:	0f 85 e6 fd ff ff    	jne    46ab85 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x815>
  46ad9f:	eb 46                	jmp    46ade7 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  46ada1:	41 c1 e1 07          	shl    $0x7,%r9d
  46ada5:	49 63 c1             	movslq %r9d,%rax
  46ada8:	62 71 7c 48 11 74 83 	vmovups %zmm14,0x80(%rbx,%rax,4)
  46adaf:	02 
  46adb0:	62 71 7c 48 11 7c 83 	vmovups %zmm15,0x40(%rbx,%rax,4)
  46adb7:	01 
  46adb8:	62 71 7c 48 11 2c 83 	vmovups %zmm13,(%rbx,%rax,4)
  46adbf:	62 71 7c 48 11 64 83 	vmovups %zmm12,0xc0(%rbx,%rax,4)
  46adc6:	03 
  46adc7:	62 71 7c 48 11 5c 83 	vmovups %zmm11,0x100(%rbx,%rax,4)
  46adce:	04 
  46adcf:	62 71 7c 48 11 54 83 	vmovups %zmm10,0x140(%rbx,%rax,4)
  46add6:	05 
  46add7:	62 71 7c 48 11 4c 83 	vmovups %zmm9,0x180(%rbx,%rax,4)
  46adde:	06 
  46addf:	62 f1 7c 48 11 6c 83 	vmovups %zmm5,0x1c0(%rbx,%rax,4)
  46ade6:	07 
  46ade7:	5b                   	pop    %rbx
  46ade8:	41 5c                	pop    %r12
  46adea:	41 5d                	pop    %r13
  46adec:	41 5e                	pop    %r14
  46adee:	41 5f                	pop    %r15
  46adf0:	5d                   	pop    %rbp
  46adf1:	c5 f8 77             	vzeroupper 
  46adf4:	c3                   	retq   
  46adf5:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
  46adfc:	00 00 00 
  46adff:	90                   	nop

