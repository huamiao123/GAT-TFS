00000000004631e0 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)>:
  4631e0:	55                   	push   %rbp
  4631e1:	41 57                	push   %r15
  4631e3:	41 56                	push   %r14
  4631e5:	41 55                	push   %r13
  4631e7:	41 54                	push   %r12
  4631e9:	53                   	push   %rbx
  4631ea:	0f b6 44 24 50       	movzbl 0x50(%rsp),%eax
  4631ef:	8b 5c 24 48          	mov    0x48(%rsp),%ebx
  4631f3:	8b 6c 24 40          	mov    0x40(%rsp),%ebp
  4631f7:	44 8b 6c 24 38       	mov    0x38(%rsp),%r13d
  4631fc:	4c 8b 54 24 78       	mov    0x78(%rsp),%r10
  463201:	4d 8b 32             	mov    (%r10),%r14
  463204:	4d 8d 5e 01          	lea    0x1(%r14),%r11
  463208:	4d 89 1a             	mov    %r11,(%r10)
  46320b:	45 0f b6 1e          	movzbl (%r14),%r11d
  46320f:	4d 89 dc             	mov    %r11,%r12
  463212:	41 83 c3 03          	add    $0x3,%r11d
  463216:	41 c1 eb 02          	shr    $0x2,%r11d
  46321a:	4c 89 74 24 e0       	mov    %r14,-0x20(%rsp)
  46321f:	4f 8d 5c 1e 01       	lea    0x1(%r14,%r11,1),%r11
  463224:	4d 89 1a             	mov    %r11,(%r10)
  463227:	4d 63 d0             	movslq %r8d,%r10
  46322a:	4c 89 54 24 c8       	mov    %r10,-0x38(%rsp)
  46322f:	46 8b 14 91          	mov    (%rcx,%r10,4),%r10d
  463233:	41 39 da             	cmp    %ebx,%r10d
  463236:	44 0f 43 d3          	cmovae %ebx,%r10d
  46323a:	41 39 ea             	cmp    %ebp,%r10d
  46323d:	41 89 eb             	mov    %ebp,%r11d
  463240:	44 89 54 24 c4       	mov    %r10d,-0x3c(%rsp)
  463245:	45 0f 42 da          	cmovb  %r10d,%r11d
  463249:	44 89 5c 24 c0       	mov    %r11d,-0x40(%rsp)
  46324e:	45 39 eb             	cmp    %r13d,%r11d
  463251:	41 89 c3             	mov    %eax,%r11d
  463254:	41 0f 97 c6          	seta   %r14b
  463258:	45 08 f3             	or     %r14b,%r11b
  46325b:	4c 8b 54 24 58       	mov    0x58(%rsp),%r10
  463260:	45 85 c9             	test   %r9d,%r9d
  463263:	78 61                	js     4632c6 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xe6>
  463265:	45 89 cf             	mov    %r9d,%r15d
  463268:	42 8b 0c b9          	mov    (%rcx,%r15,4),%ecx
  46326c:	39 d9                	cmp    %ebx,%ecx
  46326e:	0f 42 d9             	cmovb  %ecx,%ebx
  463271:	39 eb                	cmp    %ebp,%ebx
  463273:	89 5c 24 b0          	mov    %ebx,-0x50(%rsp)
  463277:	0f 42 eb             	cmovb  %ebx,%ebp
  46327a:	89 6c 24 b4          	mov    %ebp,-0x4c(%rsp)
  46327e:	44 39 ed             	cmp    %r13d,%ebp
  463281:	0f 97 c1             	seta   %cl
  463284:	08 c1                	or     %al,%cl
  463286:	45 84 db             	test   %r11b,%r11b
  463289:	0f 84 89 00 00 00    	je     463318 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x138>
  46328f:	84 c0                	test   %al,%al
  463291:	0f 84 8e 00 00 00    	je     463325 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x145>
  463297:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  46329c:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  4632a1:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  4632a5:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  4632a9:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  4632ad:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  4632b1:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  4632b5:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  4632b9:	84 c9                	test   %cl,%cl
  4632bb:	0f 85 b7 00 00 00    	jne    463378 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x198>
  4632c1:	e9 f9 00 00 00       	jmpq   4633bf <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x1df>
  4632c6:	45 84 db             	test   %r11b,%r11b
  4632c9:	0f 84 86 09 00 00    	je     463c55 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa75>
  4632cf:	84 c0                	test   %al,%al
  4632d1:	0f 84 8f 01 00 00    	je     463466 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x286>
  4632d7:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  4632dc:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  4632e1:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  4632e5:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  4632e9:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  4632ed:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  4632f1:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  4632f5:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  4632f9:	c6 44 24 af 01       	movb   $0x1,-0x51(%rsp)
  4632fe:	44 39 6c 24 c0       	cmp    %r13d,-0x40(%rsp)
  463303:	0f 87 b4 01 00 00    	ja     4634bd <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x2dd>
  463309:	c7 44 24 b0 00 00 00 	movl   $0x0,-0x50(%rsp)
  463310:	00 
  463311:	31 c0                	xor    %eax,%eax
  463313:	e9 c2 01 00 00       	jmpq   4634da <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x2fa>
  463318:	84 c9                	test   %cl,%cl
  46331a:	0f 84 35 09 00 00    	je     463c55 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa75>
  463320:	45 31 f6             	xor    %r14d,%r14d
  463323:	eb 53                	jmp    463378 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x198>
  463325:	44 89 db             	mov    %r11d,%ebx
  463328:	45 89 c3             	mov    %r8d,%r11d
  46332b:	41 c1 e3 07          	shl    $0x7,%r11d
  46332f:	4d 63 db             	movslq %r11d,%r11
  463332:	62 91 7c 48 10 44 9a 	vmovups 0x1c0(%r10,%r11,4),%zmm0
  463339:	07 
  46333a:	62 91 7c 48 10 4c 9a 	vmovups 0x180(%r10,%r11,4),%zmm1
  463341:	06 
  463342:	62 91 7c 48 10 54 9a 	vmovups 0x140(%r10,%r11,4),%zmm2
  463349:	05 
  46334a:	62 91 7c 48 10 5c 9a 	vmovups 0x100(%r10,%r11,4),%zmm3
  463351:	04 
  463352:	62 11 7c 48 10 0c 9a 	vmovups (%r10,%r11,4),%zmm9
  463359:	62 11 7c 48 10 44 9a 	vmovups 0x40(%r10,%r11,4),%zmm8
  463360:	01 
  463361:	62 91 7c 48 10 74 9a 	vmovups 0x80(%r10,%r11,4),%zmm6
  463368:	02 
  463369:	62 91 7c 48 10 64 9a 	vmovups 0xc0(%r10,%r11,4),%zmm4
  463370:	03 
  463371:	41 89 db             	mov    %ebx,%r11d
  463374:	84 c9                	test   %cl,%cl
  463376:	74 47                	je     4633bf <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x1df>
  463378:	b1 01                	mov    $0x1,%cl
  46337a:	89 4c 24 b8          	mov    %ecx,-0x48(%rsp)
  46337e:	84 c0                	test   %al,%al
  463380:	44 88 5c 24 af       	mov    %r11b,-0x51(%rsp)
  463385:	74 54                	je     4633db <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x1fb>
  463387:	c4 41 10 57 ed       	vxorps %xmm13,%xmm13,%xmm13
  46338c:	c4 41 00 57 ff       	vxorps %xmm15,%xmm15,%xmm15
  463391:	c4 41 08 57 f6       	vxorps %xmm14,%xmm14,%xmm14
  463396:	c4 41 18 57 e4       	vxorps %xmm12,%xmm12,%xmm12
  46339b:	c4 41 20 57 db       	vxorps %xmm11,%xmm11,%xmm11
  4633a0:	c4 41 28 57 d2       	vxorps %xmm10,%xmm10,%xmm10
  4633a5:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  4633a9:	c5 d0 57 ed          	vxorps %xmm5,%xmm5,%xmm5
  4633ad:	45 31 db             	xor    %r11d,%r11d
  4633b0:	b8 00 00 00 00       	mov    $0x0,%eax
  4633b5:	45 84 f6             	test   %r14b,%r14b
  4633b8:	75 75                	jne    46342f <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x24f>
  4633ba:	e9 85 00 00 00       	jmpq   463444 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x264>
  4633bf:	c6 44 24 af 01       	movb   $0x1,-0x51(%rsp)
  4633c4:	c7 44 24 b8 00 00 00 	movl   $0x0,-0x48(%rsp)
  4633cb:	00 
  4633cc:	45 31 db             	xor    %r11d,%r11d
  4633cf:	b8 00 00 00 00       	mov    $0x0,%eax
  4633d4:	45 84 f6             	test   %r14b,%r14b
  4633d7:	75 56                	jne    46342f <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x24f>
  4633d9:	eb 69                	jmp    463444 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x264>
  4633db:	44 89 c8             	mov    %r9d,%eax
  4633de:	c1 e0 07             	shl    $0x7,%eax
  4633e1:	48 98                	cltq   
  4633e3:	62 d1 7c 48 10 6c 82 	vmovups 0x1c0(%r10,%rax,4),%zmm5
  4633ea:	07 
  4633eb:	62 d1 7c 48 10 7c 82 	vmovups 0x180(%r10,%rax,4),%zmm7
  4633f2:	06 
  4633f3:	62 51 7c 48 10 54 82 	vmovups 0x140(%r10,%rax,4),%zmm10
  4633fa:	05 
  4633fb:	62 51 7c 48 10 5c 82 	vmovups 0x100(%r10,%rax,4),%zmm11
  463402:	04 
  463403:	62 51 7c 48 10 2c 82 	vmovups (%r10,%rax,4),%zmm13
  46340a:	62 51 7c 48 10 7c 82 	vmovups 0x40(%r10,%rax,4),%zmm15
  463411:	01 
  463412:	62 51 7c 48 10 74 82 	vmovups 0x80(%r10,%rax,4),%zmm14
  463419:	02 
  46341a:	62 51 7c 48 10 64 82 	vmovups 0xc0(%r10,%rax,4),%zmm12
  463421:	03 
  463422:	45 31 db             	xor    %r11d,%r11d
  463425:	b8 00 00 00 00       	mov    $0x0,%eax
  46342a:	45 84 f6             	test   %r14b,%r14b
  46342d:	74 15                	je     463444 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x264>
  46342f:	48 8b 44 24 c8       	mov    -0x38(%rsp),%rax
  463434:	8b 04 82             	mov    (%rdx,%rax,4),%eax
  463437:	44 89 e9             	mov    %r13d,%ecx
  46343a:	48 01 c1             	add    %rax,%rcx
  46343d:	48 8b 47 18          	mov    0x18(%rdi),%rax
  463441:	8b 04 88             	mov    (%rax,%rcx,4),%eax
  463444:	44 39 6c 24 b4       	cmp    %r13d,-0x4c(%rsp)
  463449:	0f 86 9e 00 00 00    	jbe    4634ed <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x30d>
  46344f:	42 8b 0c ba          	mov    (%rdx,%r15,4),%ecx
  463453:	45 89 ea             	mov    %r13d,%r10d
  463456:	49 01 ca             	add    %rcx,%r10
  463459:	48 8b 4f 18          	mov    0x18(%rdi),%rcx
  46345d:	46 8b 1c 91          	mov    (%rcx,%r10,4),%r11d
  463461:	e9 87 00 00 00       	jmpq   4634ed <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x30d>
  463466:	44 89 c0             	mov    %r8d,%eax
  463469:	c1 e0 07             	shl    $0x7,%eax
  46346c:	48 98                	cltq   
  46346e:	62 d1 7c 48 10 44 82 	vmovups 0x1c0(%r10,%rax,4),%zmm0
  463475:	07 
  463476:	62 d1 7c 48 10 4c 82 	vmovups 0x180(%r10,%rax,4),%zmm1
  46347d:	06 
  46347e:	62 d1 7c 48 10 54 82 	vmovups 0x140(%r10,%rax,4),%zmm2
  463485:	05 
  463486:	62 d1 7c 48 10 5c 82 	vmovups 0x100(%r10,%rax,4),%zmm3
  46348d:	04 
  46348e:	62 51 7c 48 10 0c 82 	vmovups (%r10,%rax,4),%zmm9
  463495:	62 51 7c 48 10 44 82 	vmovups 0x40(%r10,%rax,4),%zmm8
  46349c:	01 
  46349d:	62 d1 7c 48 10 74 82 	vmovups 0x80(%r10,%rax,4),%zmm6
  4634a4:	02 
  4634a5:	62 d1 7c 48 10 64 82 	vmovups 0xc0(%r10,%rax,4),%zmm4
  4634ac:	03 
  4634ad:	c6 44 24 af 01       	movb   $0x1,-0x51(%rsp)
  4634b2:	44 39 6c 24 c0       	cmp    %r13d,-0x40(%rsp)
  4634b7:	0f 86 4c fe ff ff    	jbe    463309 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x129>
  4634bd:	48 8b 44 24 c8       	mov    -0x38(%rsp),%rax
  4634c2:	8b 04 82             	mov    (%rdx,%rax,4),%eax
  4634c5:	44 89 e9             	mov    %r13d,%ecx
  4634c8:	48 01 c1             	add    %rax,%rcx
  4634cb:	48 8b 47 18          	mov    0x18(%rdi),%rax
  4634cf:	8b 04 88             	mov    (%rax,%rcx,4),%eax
  4634d2:	c7 44 24 b0 00 00 00 	movl   $0x0,-0x50(%rsp)
  4634d9:	00 
  4634da:	c7 44 24 b4 00 00 00 	movl   $0x0,-0x4c(%rsp)
  4634e1:	00 
  4634e2:	c7 44 24 b8 00 00 00 	movl   $0x0,-0x48(%rsp)
  4634e9:	00 
  4634ea:	45 31 db             	xor    %r11d,%r11d
  4634ed:	4c 89 e1             	mov    %r12,%rcx
  4634f0:	44 89 44 24 bc       	mov    %r8d,-0x44(%rsp)
  4634f5:	85 c9                	test   %ecx,%ecx
  4634f7:	44 89 4c 24 d4       	mov    %r9d,-0x2c(%rsp)
  4634fc:	0f 84 05 03 00 00    	je     463807 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x627>
  463502:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  463506:	4d 63 c1             	movslq %r9d,%r8
  463509:	41 89 c9             	mov    %ecx,%r9d
  46350c:	49 ff c9             	dec    %r9
  46350f:	49 c1 e9 02          	shr    $0x2,%r9
  463513:	4c 89 4c 24 d8       	mov    %r9,-0x28(%rsp)
  463518:	45 31 d2             	xor    %r10d,%r10d
  46351b:	45 89 ec             	mov    %r13d,%r12d
  46351e:	44 8b 4c 24 b0       	mov    -0x50(%rsp),%r9d
  463523:	eb 28                	jmp    46354d <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x36d>
  463525:	66 66 2e 0f 1f 84 00 	data16 nopw %cs:0x0(%rax,%rax,1)
  46352c:	00 00 00 00 
  463530:	48 8b 4c 24 f8       	mov    -0x8(%rsp),%rcx
  463535:	83 c1 fc             	add    $0xfffffffc,%ecx
  463538:	4c 8b 54 24 f0       	mov    -0x10(%rsp),%r10
  46353d:	4c 3b 54 24 d8       	cmp    -0x28(%rsp),%r10
  463542:	4c 8b 54 24 e8       	mov    -0x18(%rsp),%r10
  463547:	0f 84 ba 02 00 00    	je     463807 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x627>
  46354d:	83 f9 04             	cmp    $0x4,%ecx
  463550:	41 bf 04 00 00 00    	mov    $0x4,%r15d
  463556:	48 89 4c 24 f8       	mov    %rcx,-0x8(%rsp)
  46355b:	44 0f 42 f9          	cmovb  %ecx,%r15d
  46355f:	41 83 ff 01          	cmp    $0x1,%r15d
  463563:	41 83 d7 00          	adc    $0x0,%r15d
  463567:	49 8d 4a 01          	lea    0x1(%r10),%rcx
  46356b:	48 89 4c 24 e8       	mov    %rcx,-0x18(%rsp)
  463570:	48 8b 4c 24 e0       	mov    -0x20(%rsp),%rcx
  463575:	4c 89 54 24 f0       	mov    %r10,-0x10(%rsp)
  46357a:	46 0f b6 74 11 01    	movzbl 0x1(%rcx,%r10,1),%r14d
  463580:	eb 1a                	jmp    46359c <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3bc>
  463582:	66 66 66 66 66 2e 0f 	data16 data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  463589:	1f 84 00 00 00 00 00 
  463590:	41 89 cc             	mov    %ecx,%r12d
  463593:	41 c0 ee 02          	shr    $0x2,%r14b
  463597:	41 ff cf             	dec    %r15d
  46359a:	74 94                	je     463530 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x350>
  46359c:	41 8d 6d 01          	lea    0x1(%r13),%ebp
  4635a0:	41 f6 c6 01          	test   $0x1,%r14b
  4635a4:	74 36                	je     4635dc <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3fc>
  4635a6:	3b 6c 24 c4          	cmp    -0x3c(%rsp),%ebp
  4635aa:	73 30                	jae    4635dc <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3fc>
  4635ac:	48 8b 4c 24 c8       	mov    -0x38(%rsp),%rcx
  4635b1:	8b 0c 8a             	mov    (%rdx,%rcx,4),%ecx
  4635b4:	45 89 ea             	mov    %r13d,%r10d
  4635b7:	49 01 ca             	add    %rcx,%r10
  4635ba:	42 8b 4c 97 04       	mov    0x4(%rdi,%r10,4),%ecx
  4635bf:	48 c1 e1 08          	shl    $0x8,%rcx
  4635c3:	0f 18 0c 0e          	prefetcht0 (%rsi,%rcx,1)
  4635c7:	0f 18 4c 0e 40       	prefetcht0 0x40(%rsi,%rcx,1)
  4635cc:	0f 18 8c 0e 80 00 00 	prefetcht0 0x80(%rsi,%rcx,1)
  4635d3:	00 
  4635d4:	0f 18 8c 0e c0 00 00 	prefetcht0 0xc0(%rsi,%rcx,1)
  4635db:	00 
  4635dc:	41 8d 4c 24 01       	lea    0x1(%r12),%ecx
  4635e1:	41 f6 c6 02          	test   $0x2,%r14b
  4635e5:	74 35                	je     46361c <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x43c>
  4635e7:	44 39 c9             	cmp    %r9d,%ecx
  4635ea:	73 30                	jae    46361c <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x43c>
  4635ec:	46 8b 14 82          	mov    (%rdx,%r8,4),%r10d
  4635f0:	44 89 e3             	mov    %r12d,%ebx
  4635f3:	4c 01 d3             	add    %r10,%rbx
  4635f6:	44 8b 54 9f 04       	mov    0x4(%rdi,%rbx,4),%r10d
  4635fb:	49 c1 e2 08          	shl    $0x8,%r10
  4635ff:	42 0f 18 0c 16       	prefetcht0 (%rsi,%r10,1)
  463604:	42 0f 18 4c 16 40    	prefetcht0 0x40(%rsi,%r10,1)
  46360a:	42 0f 18 8c 16 80 00 	prefetcht0 0x80(%rsi,%r10,1)
  463611:	00 00 
  463613:	42 0f 18 8c 16 c0 00 	prefetcht0 0xc0(%rsi,%r10,1)
  46361a:	00 00 
  46361c:	41 f6 c6 01          	test   $0x1,%r14b
  463620:	75 2e                	jne    463650 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x470>
  463622:	41 f6 c6 02          	test   $0x2,%r14b
  463626:	0f 85 dc 00 00 00    	jne    463708 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x528>
  46362c:	41 f6 c6 01          	test   $0x1,%r14b
  463630:	0f 85 8a 01 00 00    	jne    4637c0 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x5e0>
  463636:	41 f6 c6 02          	test   $0x2,%r14b
  46363a:	0f 84 53 ff ff ff    	je     463593 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3b3>
  463640:	e9 a5 01 00 00       	jmpq   4637ea <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x60a>
  463645:	66 66 2e 0f 1f 84 00 	data16 nopw %cs:0x0(%rax,%rax,1)
  46364c:	00 00 00 00 
  463650:	41 89 c2             	mov    %eax,%r10d
  463653:	49 c1 e2 08          	shl    $0x8,%r10
  463657:	62 a2 7d 48 33 44 16 	vpmovzxwd 0xa0(%rsi,%r10,1),%zmm16
  46365e:	05 
  46365f:	62 a2 7d 48 33 4c 16 	vpmovzxwd 0x80(%rsi,%r10,1),%zmm17
  463666:	04 
  463667:	62 a2 7d 48 33 54 16 	vpmovzxwd 0x60(%rsi,%r10,1),%zmm18
  46366e:	03 
  46366f:	62 a2 7d 48 33 5c 16 	vpmovzxwd 0x40(%rsi,%r10,1),%zmm19
  463676:	02 
  463677:	62 a2 7d 48 33 64 16 	vpmovzxwd 0x20(%rsi,%r10,1),%zmm20
  46367e:	01 
  46367f:	62 a2 7d 48 33 2c 16 	vpmovzxwd (%rsi,%r10,1),%zmm21
  463686:	62 a2 7d 48 33 74 16 	vpmovzxwd 0xc0(%rsi,%r10,1),%zmm22
  46368d:	06 
  46368e:	62 a2 7d 48 33 7c 16 	vpmovzxwd 0xe0(%rsi,%r10,1),%zmm23
  463695:	07 
  463696:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  46369d:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  4636a4:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  4636ab:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  4636b2:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  4636b9:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  4636c0:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  4636c7:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  4636ce:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  4636d4:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  4636da:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  4636e0:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  4636e6:	62 31 3c 48 58 c4    	vaddps %zmm20,%zmm8,%zmm8
  4636ec:	62 31 34 48 58 cd    	vaddps %zmm21,%zmm9,%zmm9
  4636f2:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  4636f8:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  4636fe:	41 f6 c6 02          	test   $0x2,%r14b
  463702:	0f 84 24 ff ff ff    	je     46362c <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x44c>
  463708:	45 89 da             	mov    %r11d,%r10d
  46370b:	49 c1 e2 08          	shl    $0x8,%r10
  46370f:	62 a2 7d 48 33 44 16 	vpmovzxwd 0xa0(%rsi,%r10,1),%zmm16
  463716:	05 
  463717:	62 a2 7d 48 33 4c 16 	vpmovzxwd 0x80(%rsi,%r10,1),%zmm17
  46371e:	04 
  46371f:	62 a2 7d 48 33 54 16 	vpmovzxwd 0x60(%rsi,%r10,1),%zmm18
  463726:	03 
  463727:	62 a2 7d 48 33 5c 16 	vpmovzxwd 0x40(%rsi,%r10,1),%zmm19
  46372e:	02 
  46372f:	62 a2 7d 48 33 64 16 	vpmovzxwd 0x20(%rsi,%r10,1),%zmm20
  463736:	01 
  463737:	62 a2 7d 48 33 2c 16 	vpmovzxwd (%rsi,%r10,1),%zmm21
  46373e:	62 a2 7d 48 33 74 16 	vpmovzxwd 0xc0(%rsi,%r10,1),%zmm22
  463745:	06 
  463746:	62 a2 7d 48 33 7c 16 	vpmovzxwd 0xe0(%rsi,%r10,1),%zmm23
  46374d:	07 
  46374e:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  463755:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  46375c:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  463763:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  46376a:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  463771:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  463778:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  46377f:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  463786:	62 31 2c 48 58 d0    	vaddps %zmm16,%zmm10,%zmm10
  46378c:	62 31 24 48 58 d9    	vaddps %zmm17,%zmm11,%zmm11
  463792:	62 31 1c 48 58 e2    	vaddps %zmm18,%zmm12,%zmm12
  463798:	62 31 0c 48 58 f3    	vaddps %zmm19,%zmm14,%zmm14
  46379e:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  4637a4:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  4637aa:	62 b1 44 48 58 fe    	vaddps %zmm22,%zmm7,%zmm7
  4637b0:	62 b1 54 48 58 ef    	vaddps %zmm23,%zmm5,%zmm5
  4637b6:	41 f6 c6 01          	test   $0x1,%r14b
  4637ba:	0f 84 76 fe ff ff    	je     463636 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x456>
  4637c0:	3b 6c 24 c0          	cmp    -0x40(%rsp),%ebp
  4637c4:	73 17                	jae    4637dd <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x5fd>
  4637c6:	48 8b 44 24 c8       	mov    -0x38(%rsp),%rax
  4637cb:	8b 04 82             	mov    (%rdx,%rax,4),%eax
  4637ce:	41 89 e9             	mov    %ebp,%r9d
  4637d1:	49 01 c1             	add    %rax,%r9
  4637d4:	42 8b 04 8f          	mov    (%rdi,%r9,4),%eax
  4637d8:	44 8b 4c 24 b0       	mov    -0x50(%rsp),%r9d
  4637dd:	41 89 ed             	mov    %ebp,%r13d
  4637e0:	41 f6 c6 02          	test   $0x2,%r14b
  4637e4:	0f 84 a9 fd ff ff    	je     463593 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3b3>
  4637ea:	3b 4c 24 b4          	cmp    -0x4c(%rsp),%ecx
  4637ee:	0f 83 9c fd ff ff    	jae    463590 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3b0>
  4637f4:	46 8b 14 82          	mov    (%rdx,%r8,4),%r10d
  4637f8:	41 89 cb             	mov    %ecx,%r11d
  4637fb:	4d 01 d3             	add    %r10,%r11
  4637fe:	46 8b 1c 9f          	mov    (%rdi,%r11,4),%r11d
  463802:	e9 89 fd ff ff       	jmpq   463590 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3b0>
  463807:	48 8b 54 24 70       	mov    0x70(%rsp),%rdx
  46380c:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  463811:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
  463816:	80 7c 24 af 00       	cmpb   $0x0,-0x51(%rsp)
  46381b:	8b 7c 24 d4          	mov    -0x2c(%rsp),%edi
  46381f:	4c 8b 44 24 58       	mov    0x58(%rsp),%r8
  463824:	0f 84 c0 01 00 00    	je     4639ea <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x80a>
  46382a:	80 7a 2a 00          	cmpb   $0x0,0x2a(%rdx)
  46382e:	0f 84 86 03 00 00    	je     463bba <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x9da>
  463834:	8b 4c 24 c4          	mov    -0x3c(%rsp),%ecx
  463838:	3b 4c 24 40          	cmp    0x40(%rsp),%ecx
  46383c:	0f 87 78 03 00 00    	ja     463bba <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x9da>
  463842:	62 d1 7d 40 72 d1 10 	vpsrld $0x10,%zmm9,%zmm16
  463849:	8b 4c 24 bc          	mov    -0x44(%rsp),%ecx
  46384d:	c1 e1 07             	shl    $0x7,%ecx
  463850:	48 63 c9             	movslq %ecx,%rcx
  463853:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463857:	62 e2 7e 48 33 04 4e 	vpmovdw %zmm16,(%rsi,%rcx,2)
  46385e:	74 1e                	je     46387e <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x69e>
  463860:	62 e1 34 58 54 05 8e 	vandps 0x4228e(%rip){1to16},%zmm9,%zmm16        # 4a5af8 <__dso_handle+0x30>
  463867:	22 04 00 
  46386a:	62 31 34 48 5c c8    	vsubps %zmm16,%zmm9,%zmm9
  463870:	62 d1 35 48 72 d1 10 	vpsrld $0x10,%zmm9,%zmm9
  463877:	62 72 7e 48 33 0c 48 	vpmovdw %zmm9,(%rax,%rcx,2)
  46387e:	62 d1 35 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm9
  463885:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463889:	62 72 7e 48 33 4c 4e 	vpmovdw %zmm9,0x20(%rsi,%rcx,2)
  463890:	01 
  463891:	74 1f                	je     4638b2 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x6d2>
  463893:	62 71 3c 58 54 0d 5b 	vandps 0x4225b(%rip){1to16},%zmm8,%zmm9        # 4a5af8 <__dso_handle+0x30>
  46389a:	22 04 00 
  46389d:	62 51 3c 48 5c c1    	vsubps %zmm9,%zmm8,%zmm8
  4638a3:	62 d1 3d 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm8
  4638aa:	62 72 7e 48 33 44 48 	vpmovdw %zmm8,0x20(%rax,%rcx,2)
  4638b1:	01 
  4638b2:	62 f1 3d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm8
  4638b9:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  4638bd:	62 72 7e 48 33 44 4e 	vpmovdw %zmm8,0x40(%rsi,%rcx,2)
  4638c4:	02 
  4638c5:	74 1f                	je     4638e6 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x706>
  4638c7:	62 71 4c 58 54 05 27 	vandps 0x42227(%rip){1to16},%zmm6,%zmm8        # 4a5af8 <__dso_handle+0x30>
  4638ce:	22 04 00 
  4638d1:	62 d1 4c 48 5c f0    	vsubps %zmm8,%zmm6,%zmm6
  4638d7:	62 f1 4d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm6
  4638de:	62 f2 7e 48 33 74 48 	vpmovdw %zmm6,0x40(%rax,%rcx,2)
  4638e5:	02 
  4638e6:	62 f1 4d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm6
  4638ed:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  4638f1:	62 f2 7e 48 33 74 4e 	vpmovdw %zmm6,0x60(%rsi,%rcx,2)
  4638f8:	03 
  4638f9:	74 1f                	je     46391a <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x73a>
  4638fb:	62 f1 5c 58 54 35 f3 	vandps 0x421f3(%rip){1to16},%zmm4,%zmm6        # 4a5af8 <__dso_handle+0x30>
  463902:	21 04 00 
  463905:	62 f1 5c 48 5c e6    	vsubps %zmm6,%zmm4,%zmm4
  46390b:	62 f1 5d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm4
  463912:	62 f2 7e 48 33 64 48 	vpmovdw %zmm4,0x60(%rax,%rcx,2)
  463919:	03 
  46391a:	62 f1 5d 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm4
  463921:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463925:	62 f2 7e 48 33 64 4e 	vpmovdw %zmm4,0x80(%rsi,%rcx,2)
  46392c:	04 
  46392d:	74 1f                	je     46394e <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x76e>
  46392f:	62 f1 64 58 54 25 bf 	vandps 0x421bf(%rip){1to16},%zmm3,%zmm4        # 4a5af8 <__dso_handle+0x30>
  463936:	21 04 00 
  463939:	62 f1 64 48 5c dc    	vsubps %zmm4,%zmm3,%zmm3
  46393f:	62 f1 65 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm3
  463946:	62 f2 7e 48 33 5c 48 	vpmovdw %zmm3,0x80(%rax,%rcx,2)
  46394d:	04 
  46394e:	62 f1 65 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm3
  463955:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463959:	62 f2 7e 48 33 5c 4e 	vpmovdw %zmm3,0xa0(%rsi,%rcx,2)
  463960:	05 
  463961:	74 1f                	je     463982 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x7a2>
  463963:	62 f1 6c 58 54 1d 8b 	vandps 0x4218b(%rip){1to16},%zmm2,%zmm3        # 4a5af8 <__dso_handle+0x30>
  46396a:	21 04 00 
  46396d:	62 f1 6c 48 5c d3    	vsubps %zmm3,%zmm2,%zmm2
  463973:	62 f1 6d 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm2
  46397a:	62 f2 7e 48 33 54 48 	vpmovdw %zmm2,0xa0(%rax,%rcx,2)
  463981:	05 
  463982:	62 f1 6d 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm2
  463989:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  46398d:	62 f2 7e 48 33 54 4e 	vpmovdw %zmm2,0xc0(%rsi,%rcx,2)
  463994:	06 
  463995:	74 1f                	je     4639b6 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x7d6>
  463997:	62 f1 74 58 54 15 57 	vandps 0x42157(%rip){1to16},%zmm1,%zmm2        # 4a5af8 <__dso_handle+0x30>
  46399e:	21 04 00 
  4639a1:	62 f1 74 48 5c ca    	vsubps %zmm2,%zmm1,%zmm1
  4639a7:	62 f1 75 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm1
  4639ae:	62 f2 7e 48 33 4c 48 	vpmovdw %zmm1,0xc0(%rax,%rcx,2)
  4639b5:	06 
  4639b6:	62 f1 75 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm1
  4639bd:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  4639c1:	62 f2 7e 48 33 4c 4e 	vpmovdw %zmm1,0xe0(%rsi,%rcx,2)
  4639c8:	07 
  4639c9:	74 1f                	je     4639ea <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x80a>
  4639cb:	62 f1 7c 58 54 0d 23 	vandps 0x42123(%rip){1to16},%zmm0,%zmm1        # 4a5af8 <__dso_handle+0x30>
  4639d2:	21 04 00 
  4639d5:	62 f1 7c 48 5c c1    	vsubps %zmm1,%zmm0,%zmm0
  4639db:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4639e2:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xe0(%rax,%rcx,2)
  4639e9:	07 
  4639ea:	80 7c 24 b8 00       	cmpb   $0x0,-0x48(%rsp)
  4639ef:	0f 84 60 02 00 00    	je     463c55 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa75>
  4639f5:	80 7a 2a 00          	cmpb   $0x0,0x2a(%rdx)
  4639f9:	0f 84 11 02 00 00    	je     463c10 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa30>
  4639ff:	8b 4c 24 b4          	mov    -0x4c(%rsp),%ecx
  463a03:	3b 4c 24 b0          	cmp    -0x50(%rsp),%ecx
  463a07:	0f 85 03 02 00 00    	jne    463c10 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa30>
  463a0d:	62 d1 7d 48 72 d5 10 	vpsrld $0x10,%zmm13,%zmm0
  463a14:	c1 e7 07             	shl    $0x7,%edi
  463a17:	48 63 cf             	movslq %edi,%rcx
  463a1a:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463a1e:	62 f2 7e 48 33 04 4e 	vpmovdw %zmm0,(%rsi,%rcx,2)
  463a25:	74 1e                	je     463a45 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x865>
  463a27:	62 f1 14 58 54 05 c7 	vandps 0x420c7(%rip){1to16},%zmm13,%zmm0        # 4a5af8 <__dso_handle+0x30>
  463a2e:	20 04 00 
  463a31:	62 f1 14 48 5c c0    	vsubps %zmm0,%zmm13,%zmm0
  463a37:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  463a3e:	62 f2 7e 48 33 04 48 	vpmovdw %zmm0,(%rax,%rcx,2)
  463a45:	62 d1 7d 48 72 d7 10 	vpsrld $0x10,%zmm15,%zmm0
  463a4c:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463a50:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x20(%rsi,%rcx,2)
  463a57:	01 
  463a58:	74 1f                	je     463a79 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x899>
  463a5a:	62 f1 04 58 54 05 94 	vandps 0x42094(%rip){1to16},%zmm15,%zmm0        # 4a5af8 <__dso_handle+0x30>
  463a61:	20 04 00 
  463a64:	62 f1 04 48 5c c0    	vsubps %zmm0,%zmm15,%zmm0
  463a6a:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  463a71:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x20(%rax,%rcx,2)
  463a78:	01 
  463a79:	62 d1 7d 48 72 d6 10 	vpsrld $0x10,%zmm14,%zmm0
  463a80:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463a84:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x40(%rsi,%rcx,2)
  463a8b:	02 
  463a8c:	74 1f                	je     463aad <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x8cd>
  463a8e:	62 f1 0c 58 54 05 60 	vandps 0x42060(%rip){1to16},%zmm14,%zmm0        # 4a5af8 <__dso_handle+0x30>
  463a95:	20 04 00 
  463a98:	62 f1 0c 48 5c c0    	vsubps %zmm0,%zmm14,%zmm0
  463a9e:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  463aa5:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x40(%rax,%rcx,2)
  463aac:	02 
  463aad:	62 d1 7d 48 72 d4 10 	vpsrld $0x10,%zmm12,%zmm0
  463ab4:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463ab8:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x60(%rsi,%rcx,2)
  463abf:	03 
  463ac0:	74 1f                	je     463ae1 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x901>
  463ac2:	62 f1 1c 58 54 05 2c 	vandps 0x4202c(%rip){1to16},%zmm12,%zmm0        # 4a5af8 <__dso_handle+0x30>
  463ac9:	20 04 00 
  463acc:	62 f1 1c 48 5c c0    	vsubps %zmm0,%zmm12,%zmm0
  463ad2:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  463ad9:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x60(%rax,%rcx,2)
  463ae0:	03 
  463ae1:	62 d1 7d 48 72 d3 10 	vpsrld $0x10,%zmm11,%zmm0
  463ae8:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463aec:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x80(%rsi,%rcx,2)
  463af3:	04 
  463af4:	74 1f                	je     463b15 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x935>
  463af6:	62 f1 24 58 54 05 f8 	vandps 0x41ff8(%rip){1to16},%zmm11,%zmm0        # 4a5af8 <__dso_handle+0x30>
  463afd:	1f 04 00 
  463b00:	62 f1 24 48 5c c0    	vsubps %zmm0,%zmm11,%zmm0
  463b06:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  463b0d:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x80(%rax,%rcx,2)
  463b14:	04 
  463b15:	62 d1 7d 48 72 d2 10 	vpsrld $0x10,%zmm10,%zmm0
  463b1c:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463b20:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xa0(%rsi,%rcx,2)
  463b27:	05 
  463b28:	74 1f                	je     463b49 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x969>
  463b2a:	62 f1 2c 58 54 05 c4 	vandps 0x41fc4(%rip){1to16},%zmm10,%zmm0        # 4a5af8 <__dso_handle+0x30>
  463b31:	1f 04 00 
  463b34:	62 f1 2c 48 5c c0    	vsubps %zmm0,%zmm10,%zmm0
  463b3a:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  463b41:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xa0(%rax,%rcx,2)
  463b48:	05 
  463b49:	62 f1 7d 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm0
  463b50:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463b54:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xc0(%rsi,%rcx,2)
  463b5b:	06 
  463b5c:	74 1f                	je     463b7d <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x99d>
  463b5e:	62 f1 44 58 54 05 90 	vandps 0x41f90(%rip){1to16},%zmm7,%zmm0        # 4a5af8 <__dso_handle+0x30>
  463b65:	1f 04 00 
  463b68:	62 f1 44 48 5c c0    	vsubps %zmm0,%zmm7,%zmm0
  463b6e:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  463b75:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xc0(%rax,%rcx,2)
  463b7c:	06 
  463b7d:	62 f1 7d 48 72 d5 10 	vpsrld $0x10,%zmm5,%zmm0
  463b84:	80 7a 28 00          	cmpb   $0x0,0x28(%rdx)
  463b88:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xe0(%rsi,%rcx,2)
  463b8f:	07 
  463b90:	0f 84 bf 00 00 00    	je     463c55 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa75>
  463b96:	62 f1 54 58 54 05 58 	vandps 0x41f58(%rip){1to16},%zmm5,%zmm0        # 4a5af8 <__dso_handle+0x30>
  463b9d:	1f 04 00 
  463ba0:	62 f1 54 48 5c c0    	vsubps %zmm0,%zmm5,%zmm0
  463ba6:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  463bad:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xe0(%rax,%rcx,2)
  463bb4:	07 
  463bb5:	e9 9b 00 00 00       	jmpq   463c55 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa75>
  463bba:	8b 4c 24 bc          	mov    -0x44(%rsp),%ecx
  463bbe:	c1 e1 07             	shl    $0x7,%ecx
  463bc1:	48 63 c9             	movslq %ecx,%rcx
  463bc4:	62 d1 7c 48 11 74 88 	vmovups %zmm6,0x80(%r8,%rcx,4)
  463bcb:	02 
  463bcc:	62 51 7c 48 11 44 88 	vmovups %zmm8,0x40(%r8,%rcx,4)
  463bd3:	01 
  463bd4:	62 51 7c 48 11 0c 88 	vmovups %zmm9,(%r8,%rcx,4)
  463bdb:	62 d1 7c 48 11 64 88 	vmovups %zmm4,0xc0(%r8,%rcx,4)
  463be2:	03 
  463be3:	62 d1 7c 48 11 5c 88 	vmovups %zmm3,0x100(%r8,%rcx,4)
  463bea:	04 
  463beb:	62 d1 7c 48 11 54 88 	vmovups %zmm2,0x140(%r8,%rcx,4)
  463bf2:	05 
  463bf3:	62 d1 7c 48 11 4c 88 	vmovups %zmm1,0x180(%r8,%rcx,4)
  463bfa:	06 
  463bfb:	62 d1 7c 48 11 44 88 	vmovups %zmm0,0x1c0(%r8,%rcx,4)
  463c02:	07 
  463c03:	80 7c 24 b8 00       	cmpb   $0x0,-0x48(%rsp)
  463c08:	0f 85 e7 fd ff ff    	jne    4639f5 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x815>
  463c0e:	eb 45                	jmp    463c55 <void gcn_extra_event_trace::trace_pair_window<false, 0>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa75>
  463c10:	c1 e7 07             	shl    $0x7,%edi
  463c13:	48 63 c7             	movslq %edi,%rax
  463c16:	62 51 7c 48 11 74 80 	vmovups %zmm14,0x80(%r8,%rax,4)
  463c1d:	02 
  463c1e:	62 51 7c 48 11 7c 80 	vmovups %zmm15,0x40(%r8,%rax,4)
  463c25:	01 
  463c26:	62 51 7c 48 11 2c 80 	vmovups %zmm13,(%r8,%rax,4)
  463c2d:	62 51 7c 48 11 64 80 	vmovups %zmm12,0xc0(%r8,%rax,4)
  463c34:	03 
  463c35:	62 51 7c 48 11 5c 80 	vmovups %zmm11,0x100(%r8,%rax,4)
  463c3c:	04 
  463c3d:	62 51 7c 48 11 54 80 	vmovups %zmm10,0x140(%r8,%rax,4)
  463c44:	05 
  463c45:	62 d1 7c 48 11 7c 80 	vmovups %zmm7,0x180(%r8,%rax,4)
  463c4c:	06 
  463c4d:	62 d1 7c 48 11 6c 80 	vmovups %zmm5,0x1c0(%r8,%rax,4)
  463c54:	07 
  463c55:	5b                   	pop    %rbx
  463c56:	41 5c                	pop    %r12
  463c58:	41 5d                	pop    %r13
  463c5a:	41 5e                	pop    %r14
  463c5c:	41 5f                	pop    %r15
  463c5e:	5d                   	pop    %rbp
  463c5f:	c5 f8 77             	vzeroupper 
  463c62:	c3                   	retq   
  463c63:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
  463c6a:	00 00 00 
  463c6d:	0f 1f 00             	nopl   (%rax)

