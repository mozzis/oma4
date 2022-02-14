.id "target" "68000"
.id "translator" "as68k.exe"
.id "date" "Nov  1 1993  18:17:12"
.id "as68k.exe sid" "@(#)as68k.PL 3.18.1.1"
.file "monitor.asm"
.seg {PSCT} %1 1 1 {}
.id "Monitor" "3,0"
.seg {OFFSET_SEG@0} %97 1 u {}
.seg {VTABLE} %98 1 u {sdata}
.abs %98 0
.seg {STACK} %140 1 1 {sdata}
.seg {POINTERS} %142 1 1 {pdata}
.seg {DATA} %151 1 1 {udata}
.seg {CODE} %166 1 1 {sprogram}
.len %1 0
.org %1 0
.abs %97 0
.len %97 0
.org %97 0
.len %98 1008
.aorg %98 0 
512 {STACK}%140 +:L
1638 {CODE}%166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
2334 %166 +:L
2334 %166 +:L
2334 %166 +:L
2334 %166 +:L
2334 %166 +:L
2334 %166 +:L
2334 %166 +:L
2334 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
1638 %166 +:L
.aorg %98 1008 
.len %140 512
.org %140 0
.org %140 512
.len %142 56
.org %142 0
.defg {FCTBLPTR} %3 %142 #0 +
256 {DATA}%151 +:W
.defg {FCTBLINX} %4 %142 #2 +
'0000'
.defg {FCCODPTR} %5 %142 #4 +
'0000'
.defg {DCTBLPTR} %6 %142 #6 +
2256 %151 +:W
.defg {DCTBLINX} %7 %142 #8 +
'0000'
.defg {DCCODPTR} %8 %142 #a +
'0000'
.defg {DACTBLPTR} %9 %142 #c +
4256 %151 +:W
.defg {DACTBLINX} %10 %142 #e +
'0000'
.defg {DACCODPTR} %11 %142 #10 +
2344 %166 +:W
.defg {LCTBLPTR} %12 %142 #12 +
6256 %151 +:W
.defg {LCTBLINX} %13 %142 #14 +
'0000'
'00800000'
'00800000'
'4f4d4134204d6f6e69746f722076657273696f6e20332e302000'
.len %151 9044
.org %151 0
.org %151 2
'ffff0000'
'00000000'
'0000'
'0000'
.org %151 254
134 %151 +:W
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
'ffffffffffffffffffffffffffffffff'
.org %151 8258
.org %151 8260
.org %151 8262
.len %166 4344
.org %166 0
.mark 403 %166 #0 +
'301c'
.mark 404 %166 #2 +
'205c'
.mark 405 %166 #4 +
'd110'
.mark 406 %166 #6 +
'3a5c'
.mark 406 %166 #8 +
'4ed5'
.mark 410 %166 #a +
'301c'
.mark 411 %166 #c +
'205c'
.mark 412 %166 #e +
'd150'
.mark 413 %166 #10 +
'3a5c'
.mark 413 %166 #12 +
'4ed5'
.mark 417 %166 #14 +
'201c'
.mark 418 %166 #16 +
'205c'
.mark 419 %166 #18 +
'd190'
.mark 420 %166 #1a +
'3a5c'
.mark 420 %166 #1c +
'4ed5'
.mark 425 %166 #1e +
'301c'
.mark 426 %166 #20 +
'205c'
.mark 427 %166 #22 +
'2050'
.mark 428 %166 #24 +
'd110'
.mark 429 %166 #26 +
'3a5c'
.mark 429 %166 #28 +
'4ed5'
.mark 433 %166 #2a +
'301c'
.mark 434 %166 #2c +
'205c'
.mark 435 %166 #2e +
'2050'
.mark 436 %166 #30 +
'd150'
.mark 437 %166 #32 +
'3a5c'
.mark 437 %166 #34 +
'4ed5'
.mark 441 %166 #36 +
'201c'
.mark 442 %166 #38 +
'205c'
.mark 443 %166 #3a +
'2050'
.mark 444 %166 #3c +
'd190'
.mark 445 %166 #3e +
'3a5c'
.mark 445 %166 #40 +
'4ed5'
.mark 449 %166 #42 +
'205c'
.mark 450 %166 #44 +
'1010'
.mark 451 %166 #46 +
'225c'
.mark 452 %166 #48 +
'd111'
.mark 453 %166 #4a +
'3a5c'
.mark 453 %166 #4c +
'4ed5'
.mark 457 %166 #4e +
'205c'
.mark 458 %166 #50 +
'3010'
.mark 459 %166 #52 +
'225c'
.mark 460 %166 #54 +
'd151'
.mark 461 %166 #56 +
'3a5c'
.mark 461 %166 #58 +
'4ed5'
.mark 465 %166 #5a +
'205c'
.mark 466 %166 #5c +
'2010'
.mark 467 %166 #5e +
'225c'
.mark 468 %166 #60 +
'd191'
.mark 469 %166 #62 +
'3a5c'
.mark 469 %166 #64 +
'4ed5'
.mark 473 %166 #66 +
'205c'
.mark 474 %166 #68 +
'1010'
.mark 475 %166 #6a +
'225c'
.mark 476 %166 #6c +
'2251'
.mark 477 %166 #6e +
'd111'
.mark 478 %166 #70 +
'3a5c'
.mark 478 %166 #72 +
'4ed5'
.mark 482 %166 #74 +
'205c'
.mark 483 %166 #76 +
'3010'
.mark 484 %166 #78 +
'225c'
.mark 485 %166 #7a +
'2251'
.mark 486 %166 #7c +
'd151'
.mark 487 %166 #7e +
'3a5c'
.mark 487 %166 #80 +
'4ed5'
.mark 491 %166 #82 +
'205c'
.mark 492 %166 #84 +
'2010'
.mark 493 %166 #86 +
'225c'
.mark 494 %166 #88 +
'2251'
.mark 495 %166 #8a +
'd191'
.mark 496 %166 #8c +
'3a5c'
.mark 496 %166 #8e +
'4ed5'
.mark 500 %166 #90 +
'205c'
.mark 501 %166 #92 +
'2050'
.mark 502 %166 #94 +
'1010'
.mark 503 %166 #96 +
'225c'
.mark 504 %166 #98 +
'd111'
.mark 505 %166 #9a +
'3a5c'
.mark 505 %166 #9c +
'4ed5'
.mark 509 %166 #9e +
'205c'
.mark 510 %166 #a0 +
'2050'
.mark 511 %166 #a2 +
'3010'
.mark 512 %166 #a4 +
'225c'
.mark 513 %166 #a6 +
'd151'
.mark 514 %166 #a8 +
'3a5c'
.mark 514 %166 #aa +
'4ed5'
.mark 518 %166 #ac +
'205c'
.mark 519 %166 #ae +
'2050'
.mark 520 %166 #b0 +
'2010'
.mark 521 %166 #b2 +
'225c'
.mark 522 %166 #b4 +
'd191'
.mark 523 %166 #b6 +
'3a5c'
.mark 523 %166 #b8 +
'4ed5'
.mark 527 %166 #ba +
'205c'
.mark 528 %166 #bc +
'2050'
.mark 529 %166 #be +
'1010'
.mark 530 %166 #c0 +
'225c'
.mark 531 %166 #c2 +
'2251'
.mark 532 %166 #c4 +
'd111'
.mark 533 %166 #c6 +
'3a5c'
.mark 533 %166 #c8 +
'4ed5'
.mark 537 %166 #ca +
'205c'
.mark 538 %166 #cc +
'2050'
.mark 539 %166 #ce +
'3010'
.mark 540 %166 #d0 +
'225c'
.mark 541 %166 #d2 +
'2251'
.mark 542 %166 #d4 +
'd151'
.mark 543 %166 #d6 +
'3a5c'
.mark 543 %166 #d8 +
'4ed5'
.mark 547 %166 #da +
'205c'
.mark 548 %166 #dc +
'2050'
.mark 549 %166 #de +
'2010'
.mark 550 %166 #e0 +
'225c'
.mark 551 %166 #e2 +
'2251'
.mark 552 %166 #e4 +
'd191'
.mark 553 %166 #e6 +
'3a5c'
.mark 553 %166 #e8 +
'4ed5'
.mark 558 %166 #ea +
'301c'
.mark 559 %166 #ec +
'205c'
.mark 560 %166 #ee +
'9110'
.mark 561 %166 #f0 +
'3a5c'
.mark 561 %166 #f2 +
'4ed5'
.mark 565 %166 #f4 +
'301c'
.mark 566 %166 #f6 +
'205c'
.mark 567 %166 #f8 +
'9150'
.mark 568 %166 #fa +
'3a5c'
.mark 568 %166 #fc +
'4ed5'
.mark 572 %166 #fe +
'201c'
.mark 573 %166 #100 +
'205c'
.mark 574 %166 #102 +
'9190'
.mark 575 %166 #104 +
'3a5c'
.mark 575 %166 #106 +
'4ed5'
.mark 579 %166 #108 +
'301c'
.mark 580 %166 #10a +
'205c'
.mark 581 %166 #10c +
'2050'
.mark 582 %166 #10e +
'9110'
.mark 583 %166 #110 +
'3a5c'
.mark 583 %166 #112 +
'4ed5'
.mark 587 %166 #114 +
'301c'
.mark 588 %166 #116 +
'205c'
.mark 589 %166 #118 +
'2050'
.mark 590 %166 #11a +
'9150'
.mark 591 %166 #11c +
'3a5c'
.mark 591 %166 #11e +
'4ed5'
.mark 595 %166 #120 +
'201c'
.mark 596 %166 #122 +
'205c'
.mark 597 %166 #124 +
'2050'
.mark 598 %166 #126 +
'9190'
.mark 599 %166 #128 +
'3a5c'
.mark 599 %166 #12a +
'4ed5'
.mark 603 %166 #12c +
'205c'
.mark 604 %166 #12e +
'1010'
.mark 605 %166 #130 +
'225c'
.mark 606 %166 #132 +
'9111'
.mark 607 %166 #134 +
'3a5c'
.mark 607 %166 #136 +
'4ed5'
.mark 611 %166 #138 +
'205c'
.mark 612 %166 #13a +
'3010'
.mark 613 %166 #13c +
'225c'
.mark 614 %166 #13e +
'9151'
.mark 615 %166 #140 +
'3a5c'
.mark 615 %166 #142 +
'4ed5'
.mark 619 %166 #144 +
'205c'
.mark 620 %166 #146 +
'2010'
.mark 621 %166 #148 +
'225c'
.mark 622 %166 #14a +
'9191'
.mark 623 %166 #14c +
'3a5c'
.mark 623 %166 #14e +
'4ed5'
.mark 627 %166 #150 +
'205c'
.mark 628 %166 #152 +
'1010'
.mark 629 %166 #154 +
'225c'
.mark 630 %166 #156 +
'2251'
.mark 631 %166 #158 +
'9111'
.mark 632 %166 #15a +
'3a5c'
.mark 632 %166 #15c +
'4ed5'
.mark 636 %166 #15e +
'205c'
.mark 637 %166 #160 +
'3010'
.mark 638 %166 #162 +
'225c'
.mark 639 %166 #164 +
'2251'
.mark 640 %166 #166 +
'9151'
.mark 641 %166 #168 +
'3a5c'
.mark 641 %166 #16a +
'4ed5'
.mark 645 %166 #16c +
'205c'
.mark 646 %166 #16e +
'2010'
.mark 647 %166 #170 +
'225c'
.mark 648 %166 #172 +
'2251'
.mark 649 %166 #174 +
'9191'
.mark 650 %166 #176 +
'3a5c'
.mark 650 %166 #178 +
'4ed5'
.mark 654 %166 #17a +
'205c'
.mark 655 %166 #17c +
'2050'
.mark 656 %166 #17e +
'1010'
.mark 657 %166 #180 +
'225c'
.mark 658 %166 #182 +
'9111'
.mark 659 %166 #184 +
'3a5c'
.mark 659 %166 #186 +
'4ed5'
.mark 663 %166 #188 +
'205c'
.mark 664 %166 #18a +
'2050'
.mark 665 %166 #18c +
'3010'
.mark 666 %166 #18e +
'225c'
.mark 667 %166 #190 +
'9151'
.mark 668 %166 #192 +
'3a5c'
.mark 668 %166 #194 +
'4ed5'
.mark 672 %166 #196 +
'205c'
.mark 673 %166 #198 +
'2050'
.mark 674 %166 #19a +
'2010'
.mark 675 %166 #19c +
'225c'
.mark 676 %166 #19e +
'9191'
.mark 677 %166 #1a0 +
'3a5c'
.mark 677 %166 #1a2 +
'4ed5'
.mark 681 %166 #1a4 +
'205c'
.mark 682 %166 #1a6 +
'2050'
.mark 683 %166 #1a8 +
'1010'
.mark 684 %166 #1aa +
'225c'
.mark 685 %166 #1ac +
'2251'
.mark 686 %166 #1ae +
'9111'
.mark 687 %166 #1b0 +
'3a5c'
.mark 687 %166 #1b2 +
'4ed5'
.mark 691 %166 #1b4 +
'205c'
.mark 692 %166 #1b6 +
'2050'
.mark 693 %166 #1b8 +
'3010'
.mark 694 %166 #1ba +
'225c'
.mark 695 %166 #1bc +
'2251'
.mark 696 %166 #1be +
'9151'
.mark 697 %166 #1c0 +
'3a5c'
.mark 697 %166 #1c2 +
'4ed5'
.mark 701 %166 #1c4 +
'205c'
.mark 702 %166 #1c6 +
'2050'
.mark 703 %166 #1c8 +
'2010'
.mark 704 %166 #1ca +
'225c'
.mark 705 %166 #1cc +
'2251'
.mark 706 %166 #1ce +
'9191'
.mark 707 %166 #1d0 +
'3a5c'
.mark 707 %166 #1d2 +
'4ed5'
.mark 711 %166 #1d4 +
'301c'
.mark 712 %166 #1d6 +
'205c'
.mark 713 %166 #1d8 +
'c110'
.mark 714 %166 #1da +
'3a5c'
.mark 714 %166 #1dc +
'4ed5'
.mark 718 %166 #1de +
'301c'
.mark 719 %166 #1e0 +
'205c'
.mark 720 %166 #1e2 +
'c150'
.mark 721 %166 #1e4 +
'3a5c'
.mark 721 %166 #1e6 +
'4ed5'
.mark 725 %166 #1e8 +
'201c'
.mark 726 %166 #1ea +
'205c'
.mark 727 %166 #1ec +
'c190'
.mark 728 %166 #1ee +
'3a5c'
.mark 728 %166 #1f0 +
'4ed5'
.mark 732 %166 #1f2 +
'205c'
.mark 733 %166 #1f4 +
'1010'
.mark 734 %166 #1f6 +
'225c'
.mark 735 %166 #1f8 +
'c191'
.mark 736 %166 #1fa +
'3a5c'
.mark 736 %166 #1fc +
'4ed5'
.mark 740 %166 #1fe +
'205c'
.mark 741 %166 #200 +
'3010'
.mark 742 %166 #202 +
'225c'
.mark 743 %166 #204 +
'c191'
.mark 744 %166 #206 +
'3a5c'
.mark 744 %166 #208 +
'4ed5'
.mark 748 %166 #20a +
'205c'
.mark 749 %166 #20c +
'2010'
.mark 750 %166 #20e +
'225c'
.mark 751 %166 #210 +
'c191'
.mark 752 %166 #212 +
'3a5c'
.mark 752 %166 #214 +
'4ed5'
.mark 756 %166 #216 +
'4a79'
0 %151 +:L
.mark 757 %166 #21c +
'6706'
.mark 759 %166 #21e +
'301c'
.mark 760 %166 #220 +
'3a5c'
.mark 760 %166 #222 +
'4ed5'
.mark 762 %166 #224 +
'3854'
.mark 763 %166 #226 +
'3a5c'
.mark 763 %166 #228 +
'4ed5'
.mark 765 %166 #22a +
'4a79'
0 %151 +:L
.mark 766 %166 #230 +
'6ef2'
.mark 767 %166 #232 +
'301c'
.mark 768 %166 #234 +
'3a5c'
.mark 768 %166 #236 +
'4ed5'
.mark 770 %166 #238 +
'4a79'
0 %151 +:L
.mark 771 %166 #23e +
'6de4'
.mark 772 %166 #240 +
'301c'
.mark 773 %166 #242 +
'3a5c'
.mark 773 %166 #244 +
'4ed5'
.mark 778 %166 #246 +
'4279'
0 %151 +:L
.mark 779 %166 #24c +
'3a5c'
.mark 779 %166 #24e +
'4ed5'
.mark 781 %166 #250 +
'33fcffff'
0 %151 +:L
.mark 782 %166 #258 +
'3a5c'
.mark 782 %166 #25a +
'4ed5'
.mark 784 %166 #25c +
'33fc0001'
0 %151 +:L
.mark 785 %166 #264 +
'3a5c'
.mark 785 %166 #266 +
'4ed5'
.mark 789 %166 #268 +
'201c'
.mark 790 %166 #26a +
'205c'
.mark 791 %166 #26c +
'b090'
.mark 792 %166 #26e +
'67d6'
.mark 793 %166 #270 +
'6eea'
.mark 794 %166 #272 +
'6ddc'
.mark 798 %166 #274 +
'301c'
.mark 799 %166 #276 +
'205c'
.mark 800 %166 #278 +
'b050'
.mark 801 %166 #27a +
'67ca'
.mark 802 %166 #27c +
'6ede'
.mark 803 %166 #27e +
'6dd0'
.mark 807 %166 #280 +
'301c'
.mark 808 %166 #282 +
'205c'
.mark 809 %166 #284 +
'b010'
.mark 810 %166 #286 +
'67be'
.mark 811 %166 #288 +
'6ed2'
.mark 812 %166 #28a +
'6dc4'
.mark 816 %166 #28c +
'205c'
.mark 817 %166 #28e +
'225c'
.mark 818 %166 #290 +
'b388'
.mark 819 %166 #292 +
'67b2'
.mark 820 %166 #294 +
'6ec6'
.mark 821 %166 #296 +
'6db8'
.mark 825 %166 #298 +
'205c'
.mark 826 %166 #29a +
'225c'
.mark 827 %166 #29c +
'b348'
.mark 828 %166 #29e +
'67a6'
.mark 829 %166 #2a0 +
'6eba'
.mark 830 %166 #2a2 +
'6dac'
.mark 834 %166 #2a4 +
'205c'
.mark 835 %166 #2a6 +
'225c'
.mark 836 %166 #2a8 +
'b308'
.mark 837 %166 #2aa +
'679a'
.mark 838 %166 #2ac +
'6eae'
.mark 839 %166 #2ae +
'6da0'
.mark 843 %166 #2b0 +
'205c'
.mark 844 %166 #2b2 +
'4a50'
.mark 845 %166 #2b4 +
'6790'
.mark 846 %166 #2b6 +
'6ea4'
.mark 847 %166 #2b8 +
'6d96'
.mark 851 %166 #2ba +
'201c'
.mark 852 %166 #2bc +
'225c'
.mark 853 %166 #2be +
'2280'
.mark 854 %166 #2c0 +
'3a5c'
.mark 854 %166 #2c2 +
'4ed5'
.mark 858 %166 #2c4 +
'301c'
.mark 859 %166 #2c6 +
'225c'
.mark 860 %166 #2c8 +
'3280'
.mark 861 %166 #2ca +
'3a5c'
.mark 861 %166 #2cc +
'4ed5'
.mark 865 %166 #2ce +
'301c'
.mark 866 %166 #2d0 +
'225c'
.mark 867 %166 #2d2 +
'1280'
.mark 868 %166 #2d4 +
'3a5c'
.mark 868 %166 #2d6 +
'4ed5'
.mark 872 %166 #2d8 +
'201c'
.mark 873 %166 #2da +
'225c'
.mark 874 %166 #2dc +
'2251'
.mark 875 %166 #2de +
'2280'
.mark 876 %166 #2e0 +
'3a5c'
.mark 876 %166 #2e2 +
'4ed5'
.mark 880 %166 #2e4 +
'301c'
.mark 881 %166 #2e6 +
'225c'
.mark 882 %166 #2e8 +
'2251'
.mark 883 %166 #2ea +
'3280'
.mark 884 %166 #2ec +
'3a5c'
.mark 884 %166 #2ee +
'4ed5'
.mark 888 %166 #2f0 +
'301c'
.mark 889 %166 #2f2 +
'225c'
.mark 890 %166 #2f4 +
'2251'
.mark 891 %166 #2f6 +
'1280'
.mark 892 %166 #2f8 +
'3a5c'
.mark 892 %166 #2fa +
'4ed5'
.mark 896 %166 #2fc +
'201c'
.mark 897 %166 #2fe +
'225c'
.mark 898 %166 #300 +
'1280'
.mark 899 %166 #302 +
'e040'
.mark 900 %166 #304 +
'13400002'
.mark 901 %166 #308 +
'4840'
.mark 902 %166 #30a +
'13400004'
.mark 903 %166 #30e +
'3a5c'
.mark 903 %166 #310 +
'4ed5'
.mark 907 %166 #312 +
'301c'
.mark 908 %166 #314 +
'225c'
.mark 909 %166 #316 +
'1280'
.mark 910 %166 #318 +
'e040'
.mark 911 %166 #31a +
'13400002'
.mark 912 %166 #31e +
'3a5c'
.mark 912 %166 #320 +
'4ed5'
.mark 916 %166 #322 +
'301c'
.mark 917 %166 #324 +
'225c'
.mark 918 %166 #326 +
'1280'
.mark 919 %166 #328 +
'3a5c'
.mark 919 %166 #32a +
'4ed5'
.mark 923 %166 #32c +
'205c'
.mark 924 %166 #32e +
'225c'
.mark 925 %166 #330 +
'2290'
.mark 926 %166 #332 +
'3a5c'
.mark 926 %166 #334 +
'4ed5'
.mark 930 %166 #336 +
'205c'
.mark 931 %166 #338 +
'225c'
.mark 932 %166 #33a +
'3290'
.mark 933 %166 #33c +
'3a5c'
.mark 933 %166 #33e +
'4ed5'
.mark 937 %166 #340 +
'205c'
.mark 938 %166 #342 +
'225c'
.mark 939 %166 #344 +
'1290'
.mark 940 %166 #346 +
'3a5c'
.mark 940 %166 #348 +
'4ed5'
.mark 944 %166 #34a +
'205c'
.mark 945 %166 #34c +
'225c'
.mark 946 %166 #34e +
'2010'
.mark 947 %166 #350 +
'1280'
.mark 948 %166 #352 +
'e040'
.mark 949 %166 #354 +
'13400002'
.mark 950 %166 #358 +
'4840'
.mark 951 %166 #35a +
'13400004'
.mark 952 %166 #35e +
'3a5c'
.mark 952 %166 #360 +
'4ed5'
.mark 956 %166 #362 +
'205c'
.mark 957 %166 #364 +
'225c'
.mark 958 %166 #366 +
'3010'
.mark 959 %166 #368 +
'1280'
.mark 960 %166 #36a +
'e040'
.mark 961 %166 #36c +
'13400002'
.mark 962 %166 #370 +
'3a5c'
.mark 962 %166 #372 +
'4ed5'
.mark 966 %166 #374 +
'205c'
.mark 967 %166 #376 +
'225c'
.mark 968 %166 #378 +
'1290'
.mark 969 %166 #37a +
'3a5c'
.mark 969 %166 #37c +
'4ed5'
.mark 973 %166 #37e +
'205c'
.mark 974 %166 #380 +
'2050'
.mark 975 %166 #382 +
'225c'
.mark 976 %166 #384 +
'2290'
.mark 977 %166 #386 +
'3a5c'
.mark 977 %166 #388 +
'4ed5'
.mark 981 %166 #38a +
'205c'
.mark 982 %166 #38c +
'2050'
.mark 983 %166 #38e +
'225c'
.mark 984 %166 #390 +
'3290'
.mark 985 %166 #392 +
'3a5c'
.mark 985 %166 #394 +
'4ed5'
.mark 989 %166 #396 +
'205c'
.mark 990 %166 #398 +
'2050'
.mark 991 %166 #39a +
'225c'
.mark 992 %166 #39c +
'1290'
.mark 993 %166 #39e +
'3a5c'
.mark 993 %166 #3a0 +
'4ed5'
.mark 997 %166 #3a2 +
'205c'
.mark 998 %166 #3a4 +
'2050'
.mark 999 %166 #3a6 +
'2010'
.mark 1000 %166 #3a8 +
'225c'
.mark 1001 %166 #3aa +
'1280'
.mark 1002 %166 #3ac +
'e040'
.mark 1003 %166 #3ae +
'13400002'
.mark 1004 %166 #3b2 +
'4840'
.mark 1005 %166 #3b4 +
'13400004'
.mark 1006 %166 #3b8 +
'3a5c'
.mark 1006 %166 #3ba +
'4ed5'
.mark 1010 %166 #3bc +
'205c'
.mark 1011 %166 #3be +
'2050'
.mark 1012 %166 #3c0 +
'3010'
.mark 1013 %166 #3c2 +
'225c'
.mark 1014 %166 #3c4 +
'1280'
.mark 1015 %166 #3c6 +
'e040'
.mark 1016 %166 #3c8 +
'13400002'
.mark 1017 %166 #3cc +
'3a5c'
.mark 1017 %166 #3ce +
'4ed5'
.mark 1022 %166 #3d0 +
'205c'
.mark 1023 %166 #3d2 +
'2050'
.mark 1024 %166 #3d4 +
'225c'
.mark 1025 %166 #3d6 +
'3010'
.mark 1026 %166 #3d8 +
'1280'
.mark 1027 %166 #3da +
'3a5c'
.mark 1027 %166 #3dc +
'4ed5'
.mark 1031 %166 #3de +
'205c'
.mark 1032 %166 #3e0 +
'225c'
.mark 1033 %166 #3e2 +
'2251'
.mark 1034 %166 #3e4 +
'2290'
.mark 1035 %166 #3e6 +
'3a5c'
.mark 1035 %166 #3e8 +
'4ed5'
.mark 1039 %166 #3ea +
'205c'
.mark 1040 %166 #3ec +
'225c'
.mark 1041 %166 #3ee +
'2251'
.mark 1042 %166 #3f0 +
'3290'
.mark 1043 %166 #3f2 +
'3a5c'
.mark 1043 %166 #3f4 +
'4ed5'
.mark 1047 %166 #3f6 +
'205c'
.mark 1048 %166 #3f8 +
'225c'
.mark 1049 %166 #3fa +
'2251'
.mark 1050 %166 #3fc +
'1290'
.mark 1051 %166 #3fe +
'3a5c'
.mark 1051 %166 #400 +
'4ed5'
.mark 1055 %166 #402 +
'205c'
.mark 1056 %166 #404 +
'2050'
.mark 1057 %166 #406 +
'225c'
.mark 1058 %166 #408 +
'2251'
.mark 1059 %166 #40a +
'2290'
.mark 1060 %166 #40c +
'3a5c'
.mark 1060 %166 #40e +
'4ed5'
.mark 1064 %166 #410 +
'205c'
.mark 1065 %166 #412 +
'2050'
.mark 1066 %166 #414 +
'225c'
.mark 1067 %166 #416 +
'2251'
.mark 1068 %166 #418 +
'3290'
.mark 1069 %166 #41a +
'3a5c'
.mark 1069 %166 #41c +
'4ed5'
.mark 1073 %166 #41e +
'205c'
.mark 1074 %166 #420 +
'2050'
.mark 1075 %166 #422 +
'225c'
.mark 1076 %166 #424 +
'2251'
.mark 1077 %166 #426 +
'1290'
.mark 1078 %166 #428 +
'3a5c'
.mark 1078 %166 #42a +
'4ed5'
.mark 1087 %166 #42c +
'301c'
.mark 1088 %166 #42e +
'205c'
.mark 1089 %166 #430 +
'8110'
.mark 1090 %166 #432 +
'3a5c'
.mark 1090 %166 #434 +
'4ed5'
.mark 1094 %166 #436 +
'301c'
.mark 1095 %166 #438 +
'205c'
.mark 1096 %166 #43a +
'8150'
.mark 1097 %166 #43c +
'3a5c'
.mark 1097 %166 #43e +
'4ed5'
.mark 1101 %166 #440 +
'201c'
.mark 1102 %166 #442 +
'205c'
.mark 1103 %166 #444 +
'8190'
.mark 1104 %166 #446 +
'3a5c'
.mark 1104 %166 #448 +
'4ed5'
.mark 1108 %166 #44a +
'205c'
.mark 1109 %166 #44c +
'1010'
.mark 1110 %166 #44e +
'225c'
.mark 1111 %166 #450 +
'8111'
.mark 1112 %166 #452 +
'3a5c'
.mark 1112 %166 #454 +
'4ed5'
.mark 1116 %166 #456 +
'205c'
.mark 1117 %166 #458 +
'3010'
.mark 1118 %166 #45a +
'225c'
.mark 1119 %166 #45c +
'8151'
.mark 1120 %166 #45e +
'3a5c'
.mark 1120 %166 #460 +
'4ed5'
.mark 1124 %166 #462 +
'205c'
.mark 1125 %166 #464 +
'2010'
.mark 1126 %166 #466 +
'225c'
.mark 1127 %166 #468 +
'8191'
.mark 1128 %166 #46a +
'3a5c'
.mark 1128 %166 #46c +
'4ed5'
.mark 1132 %166 #46e +
'207c00040001'
.mark 1133 %166 #474 +
'08100000'
.mark 1134 %166 #478 +
'67fa'
.mark 1135 %166 #47a +
'3a5c'
.mark 1135 %166 #47c +
'4ed5'
.mark 1139 %166 #47e +
'207c00040001'
.mark 1140 %166 #484 +
'08100001'
.mark 1141 %166 #488 +
'67fa'
.mark 1142 %166 #48a +
'3a5c'
.mark 1142 %166 #48c +
'4ed5'
.mark 1190 %166 #48e +
'389c'
.mark 1191 %166 #490 +
'3d0c'
.mark 1192 %166 #492 +
'544c'
.mark 1193 %166 #494 +
'3a5c'
.mark 1193 %166 #496 +
'4ed5'
.mark 1195 %166 #498 +
'365c'
.mark 1196 %166 #49a +
'38d3'
.mark 1197 %166 #49c +
'3d0c'
.mark 1198 %166 #49e +
'38d3'
.mark 1199 %166 #4a0 +
'3a5c'
.mark 1199 %166 #4a2 +
'4ed5'
.mark 1201 %166 #4a4 +
'365c'
.mark 1202 %166 #4a6 +
'3653'
.mark 1203 %166 #4a8 +
'38d3'
.mark 1204 %166 #4aa +
'3d0c'
.mark 1205 %166 #4ac +
'38d3'
.mark 1206 %166 #4ae +
'3a5c'
.mark 1206 %166 #4b0 +
'4ed5'
.mark 1211 %166 #4b2 +
'364c'
.mark 1212 %166 #4b4 +
'3856'
.mark 1213 %166 #4b6 +
'535c'
.mark 1214 %166 #4b8 +
'6704'
.mark 1216 %166 #4ba +
'3a5c'
.mark 1216 %166 #4bc +
'4ed5'
.mark 1218 %166 #4be +
'544e'
.mark 1219 %166 #4c0 +
'384b'
.mark 1220 %166 #4c2 +
'3a5c'
.mark 1220 %166 #4c4 +
'4ed5'
.mark 1224 %166 #4c6 +
'3854'
.mark 1225 %166 #4c8 +
'3a5c'
.mark 1225 %166 #4ca +
'4ed5'
.mark 1231 %166 #4cc +
'3a5c'
.mark 1231 %166 #4ce +
'4ed5'
'00800000'
.mark 1242 %166 #4d4 +
'205c'
.mark 1243 %166 #4d6 +
'2a18'
.mark 1244 %166 #4d8 +
'2c08'
.mark 1245 %166 #4da +
'7e0a'
.mark 1246 %166 #4dc +
'9c47'
.mark 1247 %166 #4de +
'207c00030001'
.mark 1248 %166 #4e4 +
'247c00040001'
.mark 1249 %166 #4ea +
'47f9'
1232 %166 +:L
.mark 1250 %166 #4f0 +
'2639'
22 %142 +:L
.mark 1251 %166 #4f6 +
'7200'
.mark 1253 %166 #4f8 +
'2401'
.mark 1254 %166 #4fa +
'3246'
.mark 1256 %166 #4fc +
'e24a'
.mark 1257 %166 #4fe +
'd2c7'
.mark 1258 %166 #500 +
'65fa'
.mark 1260 %166 #502 +
'08120001'
.mark 1261 %166 #506 +
'67fa'
.mark 1263 %166 #508 +
'11590002'
.mark 1264 %166 #50c +
'1099'
.mark 1265 %166 #50e +
'1159000a'
.mark 1266 %166 #512 +
'11590008'
.mark 1267 %166 #516 +
'2803'
.mark 1268 %166 #518 +
'd899'
.mark 1269 %166 #51a +
'2684'
.mark 1270 %166 #51c +
'116b00010016'
.mark 1271 %166 #522 +
'116b00020014'
.mark 1272 %166 #528 +
'116b00030012'
.mark 1274 %166 #52e +
'3251'
.mark 1275 %166 #530 +
'4a51'
.mark 1276 %166 #532 +
'672e'
.mark 1278 %166 #534 +
'08120001'
.mark 1279 %166 #538 +
'67fa'
.mark 1281 %166 #53a +
'11590002'
.mark 1282 %166 #53e +
'1099'
.mark 1283 %166 #540 +
'1159000a'
.mark 1284 %166 #544 +
'11590008'
.mark 1285 %166 #548 +
'2803'
.mark 1286 %166 #54a +
'd899'
.mark 1287 %166 #54c +
'2684'
.mark 1288 %166 #54e +
'116b00010016'
.mark 1289 %166 #554 +
'116b00020014'
.mark 1290 %166 #55a +
'116b00030012'
.mark 1292 %166 #560 +
'60ce'
.mark 1294 %166 #562 +
'5281'
.mark 1295 %166 #564 +
'5385'
.mark 1296 %166 #566 +
'6e90'
.mark 1298 %166 #568 +
'3a5c'
.mark 1298 %166 #56a +
'4ed5'
.mark 1306 %166 #56c +
'4279'
12 %151 +:L
.mark 1307 %166 #572 +
'45fa'
-1390 %151 + %166 -:I
.mark 1308 %166 #576 +
'741f'
.mark 1309 %166 #578 +
'281c'
.mark 1310 %166 #57a +
'3a3a'
-1394 %151 + %166 -:I
.mark 1311 %166 #57e +
'24ba'
-1406 %151 + %166 -:I
.mark 1312 %166 #582 +
'267c00800000'
.mark 1313 %166 #588 +
'2012'
.mark 1314 %166 #58a +
'2604'
.mark 1315 %166 #58c +
'36c0'
.mark 1316 %166 #58e +
'e398'
.mark 1317 %166 #590 +
'4a40'
.mark 1318 %166 #592 +
'6602'
.mark 1319 %166 #594 +
'e398'
.mark 1320 %166 #596 +
'5583'
.mark 1321 %166 #598 +
'6ef2'
.mark 1322 %166 #59a +
'267c00800000'
.mark 1323 %166 #5a0 +
'2012'
.mark 1324 %166 #5a2 +
'2604'
.mark 1325 %166 #5a4 +
'b05b'
.mark 1326 %166 #5a6 +
'6622'
.mark 1327 %166 #5a8 +
'e398'
.mark 1328 %166 #5aa +
'4a40'
.mark 1329 %166 #5ac +
'6602'
.mark 1330 %166 #5ae +
'e398'
.mark 1331 %166 #5b0 +
'5583'
.mark 1332 %166 #5b2 +
'6ef0'
.mark 1333 %166 #5b4 +
'2012'
.mark 1334 %166 #5b6 +
'e398'
.mark 1335 %166 #5b8 +
'2480'
.mark 1336 %166 #5ba +
'4a45'
.mark 1337 %166 #5bc +
'57caffc4'
.mark 1339 %166 #5c0 +
'5279'
12 %151 +:L
.mark 1340 %166 #5c6 +
'3a5c'
.mark 1340 %166 #5c8 +
'4ed5'
.mark 1342 %166 #5ca +
'3a5c'
.mark 1342 %166 #5cc +
'4ed5'
.mark 1349 %166 #5ce +
'245c'
.mark 1350 %166 #5d0 +
'2452'
.mark 1351 %166 #5d2 +
'265c'
.mark 1352 %166 #5d4 +
'2013'
.mark 1353 %166 #5d6 +
'429a'
.mark 1354 %166 #5d8 +
'5980'
.mark 1355 %166 #5da +
'6efa'
.mark 1356 %166 #5dc +
'3a5c'
.mark 1356 %166 #5de +
'4ed5'
.mark 1361 %166 #5e0 +
'265c'
.mark 1362 %166 #5e2 +
'260b'
.mark 1363 %166 #5e4 +
'2653'
.mark 1364 %166 #5e6 +
'244b'
.mark 1365 %166 #5e8 +
'588b'
.mark 1367 %166 #5ea +
'4280'
.mark 1368 %166 #5ec +
'225c'
.mark 1369 %166 #5ee +
'3011'
.mark 1371 %166 #5f0 +
'2200'
.mark 1372 %166 #5f2 +
'e581'
.mark 1373 %166 #5f4 +
'95c1'
.mark 1374 %166 #5f6 +
'280a'
.mark 1375 %166 #5f8 +
'e280'
.mark 1377 %166 #5fa +
'2223'
.mark 1378 %166 #5fc +
'2692'
.mark 1379 %166 #5fe +
'24c1'
.mark 1380 %166 #600 +
'51c8fff8'
.mark 1381 %166 #604 +
'2443'
.mark 1382 %166 #606 +
'2484'
.mark 1383 %166 #608 +
'3a5c'
.mark 1383 %166 #60a +
'4ed5'
.mark 1388 %166 #60c +
'207c00040001'
.mark 1389 %166 #612 +
'227c00030001'
.mark 1390 %166 #618 +
'267c00000012'
.mark 1392 %166 #61e +
'32bc0000'
.mark 1394 %166 #622 +
'08100000'
.mark 1395 %166 #626 +
'67fa'
.mark 1397 %166 #628 +
'32bc0000'
.mark 1399 %166 #62c +
'08100000'
.mark 1400 %166 #630 +
'67fa'
.mark 1402 %166 #632 +
'337cffff0004'
.mark 1403 %166 #638 +
'32bc0000'
.mark 1404 %166 #63c +
'337c00000008'
.mark 1406 %166 #642 +
'08100001'
.mark 1407 %166 #646 +
'67fa'
.mark 1409 %166 #648 +
'32bc0000'
.mark 1410 %166 #64c +
'337c00000008'
.mark 1412 %166 #652 +
'303900800008'
.mark 1413 %166 #658 +
'e340'
.mark 1414 %166 #65a +
'60000096'
.mark 1420 %166 #65e +
'50f9'
8257 %151 +:L
.mark 1421 %166 #664 +
'6066'
.mark 1427 %166 #666 +
'007c0700'
.mark 1428 %166 #66a +
'4a7900020000'
.mark 1429 %166 #670 +
'33fc000400800002'
.mark 1430 %166 #678 +
'33fc000000800004'
.mark 1432 %166 #680 +
'43fa'
6596 %151 + %166 -:I
.mark 1433 %166 #684 +
'41f80008'
.mark 1434 %166 #688 +
'7221'
.mark 1435 %166 #68a +
'20d9'
.mark 1436 %166 #68c +
'b0fc0030'
.mark 1437 %166 #690 +
'6604'
.mark 1438 %166 #692 +
'd0fc0030'
.mark 1439 %166 #696 +
'51c9fff2'
.mark 1441 %166 #69a +
'007c0700'
.mark 1442 %166 #69e +
'21fc'
1690 %166 +:L
'0004'
.mark 1443 %166 #6a6 +
'2e7c'
512 %140 +:L
.mark 1444 %166 #6ac +
'427900020000'
.mark 1445 %166 #6b2 +
'4279'
8256 %151 +:L
.mark 1446 %166 #6b8 +
'33fc000000800002'
.mark 1447 %166 #6c0 +
'33fc000000800004'
.mark 1448 %166 #6c8 +
'027cf8ff'
.mark 1452 %166 #6cc +
'41fa'
-1488 %151 + %166 -:I
.mark 1453 %166 #6d0 +
'43fa'
-1612 %151 + %166 -:I
.mark 1454 %166 #6d4 +
'3089'
.mark 1455 %166 #6d6 +
'2c49'
.mark 1457 %166 #6d8 +
'207c00040001'
.mark 1458 %166 #6de +
'227c00030001'
.mark 1459 %166 #6e4 +
'267c00000012'
.mark 1461 %166 #6ea +
'303900800008'
.mark 1462 %166 #6f0 +
'e340'
.mark 1464 %166 #6f2 +
'49fa'
-1768 %142 + %166 -:I
.mark 1465 %166 #6f6 +
'3854'
.mark 1466 %166 #6f8 +
'38740000'
.mark 1468 %166 #6fc +
'33fc000200800002'
.mark 1470 %166 #704 +
'3a5c'
.mark 1470 %166 #706 +
'4ed5'
.mark 1475 %166 #708 +
'33fc0002'
8260 %151 +:L
.mark 1476 %166 #710 +
'33fcffff00800046'
.mark 1477 %166 #718 +
'60b2'
.mark 1483 %166 #71a +
'007c0700'
.mark 1484 %166 #71e +
'4a7900020000'
.mark 1485 %166 #724 +
'33fc000400800002'
.mark 1487 %166 #72c +
'303900800000'
.mark 1488 %166 #732 +
'673e'
.mark 1490 %166 #734 +
'0c400001'
.mark 1491 %166 #738 +
'6722'
.mark 1493 %166 #73a +
'0c400003'
.mark 1494 %166 #73e +
'671c'
.mark 1496 %166 #740 +
'0c400002'
.mark 1497 %166 #744 +
'6716'
.mark 1499 %166 #746 +
'33fc000300800002'
.mark 1500 %166 #74e +
'4a7900010000'
.mark 1501 %166 #754 +
'4279'
8260 %151 +:L
.mark 1502 %166 #75a +
'4e73'
.mark 1504 %166 #75c +
'027cf8ff'
.mark 1505 %166 #760 +
'2e7c'
512 %140 +:L
.mark 1507 %166 #766 +
'4a7900010000'
.mark 1508 %166 #76c +
'4ef9'
1740 %166 +:L
.mark 1510 %166 #772 +
'4a7900800004'
.mark 1511 %166 #778 +
'67cc'
.mark 1513 %166 #77a +
'303900800006'
.mark 1514 %166 #780 +
'67c4'
.mark 1516 %166 #782 +
'0c400001'
.mark 1517 %166 #786 +
'66be'
.mark 1519 %166 #788 +
'427900800006'
.mark 1520 %166 #78e +
'427900800004'
.mark 1521 %166 #794 +
'4e73'
.mark 1534 %166 #796 +
'33fca1a100801428'
.mark 1535 %166 #79e +
'48f8ffff'
8400 %151 +:I
.mark 1536 %166 #7a4 +
'3038'
8398 %151 +:I
.mark 1537 %166 #7a8 +
'0c400021'
.mark 1538 %166 #7ac +
'6306'
.mark 1539 %166 #7ae +
'41fa'
6987 %151 + %166 -:I
.mark 1540 %166 #7b2 +
'600a'
.mark 1541 %166 #7b4 +
'e340'
.mark 1542 %166 #7b6 +
'41fa'
6998 %151 + %166 -:I
.mark 1543 %166 #7ba +
'30700000'
.mark 1544 %166 #7be +
'43fa'
6480 %151 + %166 -:I
.mark 1545 %166 #7c2 +
'7220'
.mark 1546 %166 #7c4 +
'12d8'
.mark 1547 %166 #7c6 +
'57c9fffc'
.mark 1548 %166 #7ca +
'6f08'
.mark 1549 %166 #7cc +
'12fc0020'
.mark 1550 %166 #7d0 +
'51c9fffa'
.mark 1552 %166 #7d4 +
'4e722700'
.mark 1558 %166 #7d8 +
'31fc0000'
8398 %151 +:I
.mark 1559 %166 #7de +
'60b6'
.mark 1561 %166 #7e0 +
'31fc0001'
8398 %151 +:I
.mark 1562 %166 #7e6 +
'60ae'
.mark 1564 %166 #7e8 +
'31fc0002'
8398 %151 +:I
.mark 1565 %166 #7ee +
'60a6'
.mark 1567 %166 #7f0 +
'31fc0003'
8398 %151 +:I
.mark 1568 %166 #7f6 +
'609e'
.mark 1570 %166 #7f8 +
'31fc0004'
8398 %151 +:I
.mark 1571 %166 #7fe +
'6096'
.mark 1573 %166 #800 +
'31fc0005'
8398 %151 +:I
.mark 1574 %166 #806 +
'608e'
.mark 1576 %166 #808 +
'31fc0006'
8398 %151 +:I
.mark 1577 %166 #80e +
'6086'
.mark 1579 %166 #810 +
'31fc0007'
8398 %151 +:I
.mark 1580 %166 #816 +
'6000ff7e'
.mark 1582 %166 #81a +
'31fc0008'
8398 %151 +:I
.mark 1583 %166 #820 +
'6000ff74'
.mark 1585 %166 #824 +
'31fc0009'
8398 %151 +:I
.mark 1586 %166 #82a +
'6000ff6a'
.mark 1588 %166 #82e +
'31fc000a'
8398 %151 +:I
.mark 1589 %166 #834 +
'6000ff60'
.mark 1591 %166 #838 +
'31fc000b'
8398 %151 +:I
.mark 1592 %166 #83e +
'6000ff56'
.mark 1594 %166 #842 +
'31fc000c'
8398 %151 +:I
.mark 1595 %166 #848 +
'6000ff4c'
.mark 1597 %166 #84c +
'31fc000d'
8398 %151 +:I
.mark 1598 %166 #852 +
'6000ff42'
.mark 1600 %166 #856 +
'31fc000e'
8398 %151 +:I
.mark 1601 %166 #85c +
'6000ff38'
.mark 1603 %166 #860 +
'31fc000f'
8398 %151 +:I
.mark 1604 %166 #866 +
'6000ff2e'
.mark 1606 %166 #86a +
'31fc0010'
8398 %151 +:I
.mark 1607 %166 #870 +
'6000ff24'
.mark 1609 %166 #874 +
'31fc0011'
8398 %151 +:I
.mark 1610 %166 #87a +
'6000ff1a'
.mark 1612 %166 #87e +
'31fc0012'
8398 %151 +:I
.mark 1613 %166 #884 +
'6000ff10'
.mark 1615 %166 #888 +
'31fc0013'
8398 %151 +:I
.mark 1616 %166 #88e +
'6000ff06'
.mark 1618 %166 #892 +
'31fc0014'
8398 %151 +:I
.mark 1619 %166 #898 +
'6000fefc'
.mark 1621 %166 #89c +
'31fc0015'
8398 %151 +:I
.mark 1622 %166 #8a2 +
'6000fef2'
.mark 1624 %166 #8a6 +
'31fc0016'
8398 %151 +:I
.mark 1625 %166 #8ac +
'6000fee8'
.mark 1627 %166 #8b0 +
'31fc0017'
8398 %151 +:I
.mark 1628 %166 #8b6 +
'6000fede'
.mark 1630 %166 #8ba +
'31fc0018'
8398 %151 +:I
.mark 1631 %166 #8c0 +
'6000fed4'
.mark 1633 %166 #8c4 +
'31fc0019'
8398 %151 +:I
.mark 1634 %166 #8ca +
'6000feca'
.mark 1636 %166 #8ce +
'31fc001a'
8398 %151 +:I
.mark 1637 %166 #8d4 +
'6000fec0'
.mark 1639 %166 #8d8 +
'31fc001b'
8398 %151 +:I
.mark 1640 %166 #8de +
'6000feb6'
.mark 1642 %166 #8e2 +
'31fc001c'
8398 %151 +:I
.mark 1643 %166 #8e8 +
'6000feac'
.mark 1645 %166 #8ec +
'31fc001d'
8398 %151 +:I
.mark 1646 %166 #8f2 +
'6000fea2'
.mark 1648 %166 #8f6 +
'31fc001e'
8398 %151 +:I
.mark 1649 %166 #8fc +
'6000fe98'
.mark 1651 %166 #900 +
'31fc001f'
8398 %151 +:I
.mark 1652 %166 #906 +
'6000fe8e'
.mark 1654 %166 #90a +
'31fc0020'
8398 %151 +:I
.mark 1655 %166 #910 +
'6000fe84'
.mark 1657 %166 #914 +
'31fc0021'
8398 %151 +:I
.mark 1658 %166 #91a +
'6000fe7a'
.mark 1660 %166 #91e +
'31fc0022'
8398 %151 +:I
.mark 1661 %166 #924 +
'6000fe70'
.org %151 8262
2008 %166 +:L
2016 %166 +:L
2024 %166 +:L
2032 %166 +:L
2040 %166 +:L
2048 %166 +:L
2056 %166 +:L
2064 %166 +:L
2074 %166 +:L
2084 %166 +:L
2094 %166 +:L
1818 %166 +:L
2114 %166 +:L
1818 %166 +:L
2134 %166 +:L
1818 %166 +:L
2154 %166 +:L
1818 %166 +:L
2174 %166 +:L
2184 %166 +:L
2194 %166 +:L
2204 %166 +:L
2214 %166 +:L
2224 %166 +:L
2234 %166 +:L
2244 %166 +:L
2254 %166 +:L
2264 %166 +:L
2274 %166 +:L
2284 %166 +:L
2294 %166 +:L
2304 %166 +:L
2314 %166 +:L
2324 %166 +:L
.org %151 8400
.org %151 8464
.org %151 8496
'427573206572726f7200'
'41646472657373206572726f7200'
'496c6c6567616c20696e737472756374696f6e00'
'5a65726f2064697669646500'
'43484b20696e737472756374696f6e00'
'545241505620696e7374727563746f6e00'
'50726976696c6567652076696f6c6174696f6e00'
'545241434520657863657074696f6e00'
'4c696e65203130313000'
'4c696e65203131313100'
'53707572696f757320696e7465727275707400'
'4c6576656c203120495251204175746f766563746f7200'
'4c6576656c203220495251204175746f766563746f7200'
'4c6576656c203320495251204175746f766563746f7200'
'4c6576656c203420495251204175746f766563746f7200'
'4c6576656c203520495251204175746f766563746f7200'
'4c6576656c203620495251204175746f766563746f7200'
'4c6576656c203720495251204175746f766563746f7200'
'5472617020233000'
'5472617020233100'
'5472617020233200'
'5472617020233300'
'5472617020233400'
'5472617020233500'
'5472617020233600'
'5472617020233700'
'5472617020233800'
'5472617020233900'
'547261702023313000'
'547261702023313100'
'547261702023313200'
'547261702023313300'
'547261702023313400'
'547261702023313500'
'457863657074696f6e204572726f7200'
'00'
'0000'
8496 %151 +:W
8506 %151 +:W
8520 %151 +:W
8540 %151 +:W
8552 %151 +:W
8568 %151 +:W
8585 %151 +:W
8605 %151 +:W
8621 %151 +:W
8631 %151 +:W
8641 %151 +:W
8660 %151 +:W
8683 %151 +:W
8706 %151 +:W
8729 %151 +:W
8752 %151 +:W
8775 %151 +:W
8798 %151 +:W
8821 %151 +:W
8829 %151 +:W
8837 %151 +:W
8845 %151 +:W
8853 %151 +:W
8861 %151 +:W
8869 %151 +:W
8877 %151 +:W
8885 %151 +:W
8893 %151 +:W
8901 %151 +:W
8910 %151 +:W
8919 %151 +:W
8928 %151 +:W
8937 %151 +:W
8946 %151 +:W
8955 %151 +:W
.org %166 2344
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
1228 %166 +:W
.dcl 0 {monitor.asm} m 
.bgn 
.dcl 1 {} r 0 f 2
.bgn %98 0 +
.dcl 2 {PSCT} l %1 #0 +
.dcl 3 {FCTBLPTR} l %142 #0 +
.dcl 4 {FCTBLINX} l %142 #2 +
.dcl 5 {FCCODPTR} l %142 #4 +
.dcl 6 {DCTBLPTR} l %142 #6 +
.dcl 7 {DCTBLINX} l %142 #8 +
.dcl 8 {DCCODPTR} l %142 #a +
.dcl 9 {DACTBLPTR} l %142 #c +
.dcl 10 {DACTBLINX} l %142 #e +
.dcl 11 {DACCODPTR} l %142 #10 +
.dcl 12 {LCTBLPTR} l %142 #12 +
.dcl 13 {LCTBLINX} l %142 #14 +
.dcl 14 {MSM_Act_RQ} l #800000
.dcl 15 {MSM_Act_RQ_RPL} l #800002
.dcl 16 {MSM_Sig_PC} l #800004
.dcl 17 {MSM_Sig_PC_RPL} l #800006
.dcl 18 {MSM_Pgm_Inx} l #800008
.dcl 19 {MSM_Data_Offset} l #80000a
.dcl 20 {MSM_Bgnd_Offset} l #80000e
.dcl 21 {MSM_Data_Size} l #800012
.dcl 22 {MSM_FC_OH} l #800016
.dcl 23 {Counter_H0} l #800018
.dcl 24 {Counter_H1} l #80001a
.dcl 25 {Counter_I0} l #80001c
.dcl 26 {Counter_I1} l #80001e
.dcl 27 {Counter_J0} l #800020
.dcl 28 {Counter_J1} l #800022
.dcl 29 {Counter_K0} l #800024
.dcl 30 {Counter_K1} l #800026
.dcl 31 {Counter_L0} l #800028
.dcl 32 {Counter_L1} l #80002a
.dcl 33 {Counter_T0} l #80002c
.dcl 34 {Counter_T1} l #80002e
.dcl 35 {Counter_S0} l #800030
.dcl 36 {Counter_S1} l #800032
.dcl 37 {MSM_FC_Scan_Mode} l #800034
.dcl 38 {MSM_FC_StreakMode} l #800036
.dcl 39 {MSM_FC_StrkOTMode} l #800038
.dcl 40 {MSM_DC_Add_Mode} l #80003a
.dcl 41 {MSM_DC_Rep_Mode} l #80003c
.dcl 42 {MSM_DC_Sub_Mode} l #80003e
.dcl 43 {MSM_DC_Ign_Mode} l #800040
.dcl 44 {MSM_DC_OH} l #800042
.dcl 45 {MSM_DC_StreakMode} l #800044
.dcl 46 {MSM_DA_Complete} l #800046
.dcl 47 {MSM_Serial_Mode} l #800048
.dcl 48 {MSM_Image_Mode} l #80004a
.dcl 49 {MSM_Sort} l #80004c
.dcl 50 {MSM_Pointnum} l #80004e
.dcl 51 {MSM_SC_Address} l #800050
.dcl 52 {MSM_PIA_Out} l #800054
.dcl 53 {MSM_StreakMode} l #800056
.dcl 54 {MSM_PIA_In} l #800058
.dcl 55 {MSM_Cooler_Status} l #80005c
.dcl 56 {MSM_Shutter_ID} l #800060
.dcl 57 {MSM_Detector_ID} l #800064
.dcl 58 {VTABLE} l %98 #0 +
.dcl 59 {RESET.SP} l #0
.dcl 60 {STACK.START} l 512 %140 +
.dcl 61 {RESET.PC} l #4
.dcl 62 {STARTUP} l %166 #666 +
.dcl 63 {V.BERR} l #8
.dcl 64 {V.AERR} l #c
.dcl 65 {V.ILLG} l #10
.dcl 66 {V.ZRDV} l #14
.dcl 67 {V.CHKI} l #18
.dcl 68 {V.TRAPV} l #1c
.dcl 69 {V.PRIVIO} l #20
.dcl 70 {V.TRACE} l #24
.dcl 71 {V.L1010} l #28
.dcl 72 {V.L1111} l #2c
.dcl 73 {V.MISC1} l #30
.dcl 74 {EX_DFLT} l %166 #91e +
.dcl 75 {V.SPURI} l #50
.dcl 76 {V.IRQ1} l #54
.dcl 77 {V.IRQ2} l #58
.dcl 78 {V.IRQ3} l #5c
.dcl 79 {V.IRQ4} l #60
.dcl 80 {V.IRQ5} l #64
.dcl 81 {V.IRQ6} l #68
.dcl 82 {V.IRQ7} l #6c
.dcl 83 {V.TRAP0} l #70
.dcl 84 {V.TRAP1} l #74
.dcl 85 {V.TRAP2} l #78
.dcl 86 {V.TRAP3} l #7c
.dcl 87 {V.TRAP4} l #80
.dcl 88 {V.TRAP5} l #84
.dcl 89 {V.TRAP6} l #88
.dcl 90 {V.TRAP7} l #8c
.dcl 91 {V.TRAP8} l #90
.dcl 92 {V.TRAP9} l #94
.dcl 93 {V.TRAP10} l #98
.dcl 94 {V.TRAP11} l #9c
.dcl 95 {V.TRAP12} l #a0
.dcl 96 {V.TRAP13} l #a4
.dcl 97 {V.TRAP14} l #a8
.dcl 98 {V.TRAP15} l #ac
.dcl 99 {V.MISC2} l #b0
.dcl 100 {STACK} l %140 #0 +
.dcl 101 {stackend} l 0 %140 +
.dcl 102 {POINTERS} l %142 #0 +
.dcl 103 {FCTBL} l %151 #100 +
.dcl 104 {DCTBL} l %151 #8d0 +
.dcl 105 {DACTBL} l %151 #10a0 +
.dcl 106 {DACLIST} l %166 #928 +
.dcl 107 {LCTBL} l %151 #1870 +
.dcl 108 {DATAADDR} l %142 #16 +
.dcl 109 {BGNDADDR} l %142 #1a +
.dcl 110 {VERSION_LABEL} l %142 #1e +
.dcl 111 {DATA} l %151 #0 +
.dcl 112 {RESFLG} l %151 #0 +
.dcl 113 {INITPATT} l %151 #2 +
.dcl 114 {PATT} l %151 #6 +
.dcl 115 {QUIKTEST} l %151 #a +
.dcl 116 {SUCCESS} l %151 #c +
.dcl 117 {LOOPSTACK} l %151 #e +
.dcl 118 {LOOPTR} l %151 #fe +
.dcl 119 {FCEND} l 2256 %151 +
.dcl 120 {DMAEND} l 4256 %151 +
.dcl 121 {DACEND} l 6256 %151 +
.dcl 122 {LCEND} l 8256 %151 +
.dcl 123 {IN_DA} l %151 #2040 +
.dcl 124 {PC_INPROG} l %151 #2042 +
.dcl 125 {PC_MAILBOX} l %151 #2044 +
.dcl 126 {CODE} l %166 #0 +
.dcl 127 {_ADDIB} l %166 #0 +
.dcl 128 {_ADDIW} l %166 #a +
.dcl 129 {_ADDIL} l %166 #14 +
.dcl 130 {_ADDIBN} l %166 #1e +
.dcl 131 {_ADDIWN} l %166 #2a +
.dcl 132 {_ADDILN} l %166 #36 +
.dcl 133 {_ADDAB} l %166 #42 +
.dcl 134 {_ADDAW} l %166 #4e +
.dcl 135 {_ADDAL} l %166 #5a +
.dcl 136 {_ADDABTN} l %166 #66 +
.dcl 137 {_ADDAWTN} l %166 #74 +
.dcl 138 {_ADDALTN} l %166 #82 +
.dcl 139 {_ADDABFN} l %166 #90 +
.dcl 140 {_ADDAWFN} l %166 #9e +
.dcl 141 {_ADDALFN} l %166 #ac +
.dcl 142 {_ADDABDN} l %166 #ba +
.dcl 143 {_ADDAWDN} l %166 #ca +
.dcl 144 {_ADDALDN} l %166 #da +
.dcl 145 {_SUBIB} l %166 #ea +
.dcl 146 {_SUBIW} l %166 #f4 +
.dcl 147 {_SUBIL} l %166 #fe +
.dcl 148 {_SUBIBN} l %166 #108 +
.dcl 149 {_SUBIWN} l %166 #114 +
.dcl 150 {_SUBILN} l %166 #120 +
.dcl 151 {_SUBAB} l %166 #12c +
.dcl 152 {_SUBAW} l %166 #138 +
.dcl 153 {_SUBAL} l %166 #144 +
.dcl 154 {_SUBABTN} l %166 #150 +
.dcl 155 {_SUBAWTN} l %166 #15e +
.dcl 156 {_SUBALTN} l %166 #16c +
.dcl 157 {_SUBABFN} l %166 #17a +
.dcl 158 {_SUBAWFN} l %166 #188 +
.dcl 159 {_SUBALFN} l %166 #196 +
.dcl 160 {_SUBABDN} l %166 #1a4 +
.dcl 161 {_SUBAWDN} l %166 #1b4 +
.dcl 162 {_SUBALDN} l %166 #1c4 +
.dcl 163 {_ANDIB} l %166 #1d4 +
.dcl 164 {_ANDIW} l %166 #1de +
.dcl 165 {_ANDIL} l %166 #1e8 +
.dcl 166 {_ANDAB} l %166 #1f2 +
.dcl 167 {_ANDAW} l %166 #1fe +
.dcl 168 {_ANDAL} l %166 #20a +
.dcl 169 {_BEQ} l %166 #216 +
.dcl 170 {GO} l %166 #224 +
.dcl 171 {_BGT} l %166 #22a +
.dcl 172 {_BLT} l %166 #238 +
.dcl 173 {CLRFLG} l %166 #246 +
.dcl 174 {MINFLG} l %166 #250 +
.dcl 175 {PLSFLG} l %166 #25c +
.dcl 176 {_CMPRIL} l %166 #268 +
.dcl 177 {_CMPRIW} l %166 #274 +
.dcl 178 {_CMPRIB} l %166 #280 +
.dcl 179 {_CMPRL} l %166 #28c +
.dcl 180 {_CMPRW} l %166 #298 +
.dcl 181 {_CMPRB} l %166 #2a4 +
.dcl 182 {_TEST} l %166 #2b0 +
.dcl 183 {_MOVELI} l %166 #2ba +
.dcl 184 {_MOVEWI} l %166 #2c4 +
.dcl 185 {_MOVEBI} l %166 #2ce +
.dcl 186 {_MOVELIN} l %166 #2d8 +
.dcl 187 {_MOVEWIN} l %166 #2e4 +
.dcl 188 {_MOVEBIN} l %166 #2f0 +
.dcl 189 {_MOVELIA} l %166 #2fc +
.dcl 190 {_MOVEWIA} l %166 #312 +
.dcl 191 {_MOVEBIA} l %166 #322 +
.dcl 192 {_MOVEL} l %166 #32c +
.dcl 193 {_MOVEW} l %166 #336 +
.dcl 194 {_MOVEB} l %166 #340 +
.dcl 195 {_MOVELA} l %166 #34a +
.dcl 196 {_MOVEWA} l %166 #362 +
.dcl 197 {_MOVEBA} l %166 #374 +
.dcl 198 {_MOVELFN} l %166 #37e +
.dcl 199 {_MOVEWFN} l %166 #38a +
.dcl 200 {_MOVEBFN} l %166 #396 +
.dcl 201 {_MOVELNA} l %166 #3a2 +
.dcl 202 {_MOVEWNA} l %166 #3bc +
.dcl 203 {_MOVEBNA} l %166 #3d0 +
.dcl 204 {_MOVELTN} l %166 #3de +
.dcl 205 {_MOVEWTN} l %166 #3ea +
.dcl 206 {_MOVEBTN} l %166 #3f6 +
.dcl 207 {_MOVELDN} l %166 #402 +
.dcl 208 {_MOVEWDN} l %166 #410 +
.dcl 209 {_MOVEBDN} l %166 #41e +
.dcl 210 {_ORIB} l %166 #42c +
.dcl 211 {_ORIW} l %166 #436 +
.dcl 212 {_ORIL} l %166 #440 +
.dcl 213 {_ORAB} l %166 #44a +
.dcl 214 {_ORAW} l %166 #456 +
.dcl 215 {_ORAL} l %166 #462 +
.dcl 216 {_WAIT_FC} l %166 #46e +
.dcl 217 {WAIT1} l %166 #474 +
.dcl 218 {_WAIT_DMA} l %166 #47e +
.dcl 219 {WAIT2} l %166 #484 +
.dcl 220 {_DOI} l %166 #48e +
.dcl 221 {_DOAT} l %166 #498 +
.dcl 222 {_DOATN} l %166 #4a4 +
.dcl 223 {_LOOP} l %166 #4b2 +
.dcl 224 {NOLOOP} l %166 #4be +
.dcl 225 {_GOTO} l %166 #4c6 +
.dcl 226 {_DONOT} l %166 #4cc +
.dcl 227 {FBASE.AD} l %166 #4d0 +
.dcl 228 {_RAPSCAN} l %166 #4d4 +
.dcl 229 {SCANLOOP} l %166 #4f8 +
.dcl 230 {LOOPER} l %166 #4fc +
.dcl 231 {READ_TBL1} l %166 #502 +
.dcl 232 {READ_TBL2} l %166 #530 +
.dcl 233 {LOOP_END} l %166 #562 +
.dcl 234 {WAIT_DC} l %166 #534 +
.dcl 235 {_TESTMEM} l %166 #56c +
.dcl 236 {TMEM0} l %166 #582 +
.dcl 237 {TMEM1} l %166 #58c +
.dcl 238 {TMEM1A} l %166 #596 +
.dcl 239 {TMEM2} l %166 #5a4 +
.dcl 240 {TMEMERR} l %166 #5ca +
.dcl 241 {TMEM1B} l %166 #5b0 +
.dcl 242 {_CLRMEM} l %166 #5ce +
.dcl 243 {CLRMEM0} l %166 #5d6 +
.dcl 244 {_SORTTRK} l %166 #5e0 +
.dcl 245 {SORTTRK0} l %166 #5fa +
.dcl 246 {INIT_DAC} l %166 #60c +
.dcl 247 {DASTART1} l %166 #6f2 +
.dcl 248 {DA_SWITCH} l %166 #65e +
.dcl 249 {DA_START} l %166 #6cc +
.dcl 250 {ETABLE} l 8262 %151 +
.dcl 251 {EX_INIT} l %166 #68a +
.dcl 252 {EXINIT2} l %166 #696 +
.dcl 253 {RESTART} l %166 #69a +
.dcl 254 {DASTART2} l %166 #6ea +
.dcl 255 {_DONE} l %166 #708 +
.dcl 256 {PC_ISR} l %166 #71a +
.dcl 257 {ISR_GETRPL} l %166 #772 +
.dcl 258 {ISR_DO_ACT} l %166 #75c +
.dcl 259 {ISR_ERR} l %166 #746 +
.dcl 260 {ENDISR0} l %166 #75a +
.dcl 261 {EX_ERR} l %166 #796 +
.dcl 262 {SAVREGS} l %151 #20d0 +
.dcl 263 {ECODE} l %151 #20ce +
.dcl 264 {PMSG0} l %166 #7b4 +
.dcl 265 {DFLTMSG} l %151 #22fb +
.dcl 266 {PMSG1} l %166 #7be +
.dcl 267 {EMSGTBL} l 8974 %151 +
.dcl 268 {EMSGBUF} l %151 #2110 +
.dcl 269 {PRINTMSG} l %166 #7c4 +
.dcl 270 {EX_ERR2} l %166 #7d4 +
.dcl 271 {PMSG2} l %166 #7cc +
.dcl 272 {EX_BERR} l %166 #7d8 +
.dcl 273 {EX_AERR} l %166 #7e0 +
.dcl 274 {EX_ILLG} l %166 #7e8 +
.dcl 275 {EX_ZRDV} l %166 #7f0 +
.dcl 276 {EX_CHKI} l %166 #7f8 +
.dcl 277 {EX_TRAPV} l %166 #800 +
.dcl 278 {EX_PRIVIO} l %166 #808 +
.dcl 279 {EX_TRACE} l %166 #810 +
.dcl 280 {EX_L1010} l %166 #81a +
.dcl 281 {EX_L1111} l %166 #824 +
.dcl 282 {EX_SPURI} l %166 #82e +
.dcl 283 {EX_IRQ1} l %166 #838 +
.dcl 284 {EX_IRQ2} l %166 #842 +
.dcl 285 {EX_IRQ3} l %166 #84c +
.dcl 286 {EX_IRQ4} l %166 #856 +
.dcl 287 {EX_IRQ5} l %166 #860 +
.dcl 288 {EX_IRQ6} l %166 #86a +
.dcl 289 {EX_IRQ7} l %166 #874 +
.dcl 290 {EX_TRAP0} l %166 #87e +
.dcl 291 {EX_TRAP1} l %166 #888 +
.dcl 292 {EX_TRAP2} l %166 #892 +
.dcl 293 {EX_TRAP3} l %166 #89c +
.dcl 294 {EX_TRAP4} l %166 #8a6 +
.dcl 295 {EX_TRAP5} l %166 #8b0 +
.dcl 296 {EX_TRAP6} l %166 #8ba +
.dcl 297 {EX_TRAP7} l %166 #8c4 +
.dcl 298 {EX_TRAP8} l %166 #8ce +
.dcl 299 {EX_TRAP9} l %166 #8d8 +
.dcl 300 {EX_TRAP10} l %166 #8e2 +
.dcl 301 {EX_TRAP11} l %166 #8ec +
.dcl 302 {EX_TRAP12} l %166 #8f6 +
.dcl 303 {EX_TRAP13} l %166 #900 +
.dcl 304 {EX_TRAP14} l %166 #90a +
.dcl 305 {EX_TRAP15} l %166 #914 +
.dcl 306 {EMSGS} l 8496 %151 +
.dcl 307 {EMSG0} l %151 #2130 +
.dcl 308 {EMSG1} l %151 #213a +
.dcl 309 {EMSG2} l %151 #2148 +
.dcl 310 {EMSG3} l %151 #215c +
.dcl 311 {EMSG4} l %151 #2168 +
.dcl 312 {EMSG5} l %151 #2178 +
.dcl 313 {EMSG6} l %151 #2189 +
.dcl 314 {EMSG7} l %151 #219d +
.dcl 315 {EMSG8} l %151 #21ad +
.dcl 316 {EMSG9} l %151 #21b7 +
.dcl 317 {EMSG10} l %151 #21c1 +
.dcl 318 {EMSG11} l %151 #21d4 +
.dcl 319 {EMSG12} l %151 #21eb +
.dcl 320 {EMSG13} l %151 #2202 +
.dcl 321 {EMSG14} l %151 #2219 +
.dcl 322 {EMSG15} l %151 #2230 +
.dcl 323 {EMSG16} l %151 #2247 +
.dcl 324 {EMSG17} l %151 #225e +
.dcl 325 {EMSG18} l %151 #2275 +
.dcl 326 {EMSG19} l %151 #227d +
.dcl 327 {EMSG20} l %151 #2285 +
.dcl 328 {EMSG21} l %151 #228d +
.dcl 329 {EMSG22} l %151 #2295 +
.dcl 330 {EMSG23} l %151 #229d +
.dcl 331 {EMSG24} l %151 #22a5 +
.dcl 332 {EMSG25} l %151 #22ad +
.dcl 333 {EMSG26} l %151 #22b5 +
.dcl 334 {EMSG27} l %151 #22bd +
.dcl 335 {EMSG28} l %151 #22c5 +
.dcl 336 {EMSG29} l %151 #22ce +
.dcl 337 {EMSG30} l %151 #22d7 +
.dcl 338 {EMSG31} l %151 #22e0 +
.dcl 339 {EMSG32} l %151 #22e9 +
.dcl 340 {EMSG33} l %151 #22f2 +
.dcl 341 {VERYLAST} l 4344 %166 +
.end
.end
