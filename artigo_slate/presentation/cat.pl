$result = get_main_header_info();
$result .= paste_tunes();
$result .= fill_empty_measures();
print $result;

sub get_main_header_info {
  my %rules = (
    'note'          => sub { $has_tune = 1; q{} },
    'in_tune'       => sub { q{} },
    'in_line'       => sub { q{} },
    'in_header::M:' => sub { up_meter($sym);  toabc() },
    'in_header::L:' => sub { up_length($sym); toabc() },
    'in_header::K:' => sub { up_key($sym);    toabc() },
    'in_header::Q:' => sub { up_tempo($sym);  toabc() },
  );

  foreach my $tune (@tunes) {
    %tune_info = ();
    $main_info = abc_processor( $tune, %rules );
    last if $has_tune;
  }

  return $main_info;
}

sub paste_tunes {
  my %rules = (
    'V:'      => sub { up_tune_info(); toabc() },
    'bar'     => sub { up_measure_count(1); toabc() },
    'mrest'   => sub { up_measure_count($sym->{info}->{len}-1); toabc() },
    'w:'      => sub { toabc() },
    'staves'  => sub { q{} },
    'info'    => sub { q{} },
    'K:'      => sub { print_key($sym)    },
    'M:'      => sub { print_meter($sym)  },
    'Q:'      => sub { print_tempo($sym)  },
    'L:'      => sub { print_length($sym) },
  );

  foreach my $tune (@tunes) {
    $res .= abc_processor( $tune, %rules );
  }
  return $res;
}

sub fill_empty_measures {
  my $max = $MAX_DEFAULT;
  my $res = q{};

  # Voices ordered by number of measures (desc)
  foreach my $v (
      reverse sort { $tune_info{$a}{measures} <=> $tune_info{$b}{measures} }
      keys %tune_info
    )
  {
    if ( $tune_info{$v}{measures} > $max ) {
      $max = $tune_info{$v}{measures};
    }
    my $measures_short = $max - $tune_info{$v}{measures};
    if ($measures_short) {
      $res .= "[V:$v] Z$measures_short |\n";
    }
  }

  return $res;
}
