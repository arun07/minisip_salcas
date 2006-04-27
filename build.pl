#!/usr/bin/perl
#  build.pl - a build system management tool
#  Developed originally for the minisip project by Zachary T Welch.
#  Copyright (C) 2005-2006, Superlucidity Services, LLC
#  Released under the GNU GPL v2 license.

use strict;
use warnings;

use Cwd;
use File::Spec;
use File::Glob ':glob';
use File::Copy;
use File::Path;
use File::Basename;
use Getopt::Long qw( :config gnu_getopt );
use Pod::Usage;
use Text::Wrap;

my $app_name = basename($0);

#######
# script configuration option definitions

our $topdir = getcwd();	# path to common source directory (svn trunk)
our $confdir = undef;	# path to common build.d directory
our $builddir = undef;	# path to common build directory
our $installdir = undef; # path to common installation directory
our $prefix = undef;	# installation prefix directory
my $prefixdir = undef;	# active prefix; set to one of previous four paths
our $destdir = "";	# destination installation directory

my $bindir = undef;	#   path to installed executables
my $libdir = undef;	#   path to installed libraries
my $includedir = undef;	#   path to installed header files
my $pkgconfigdir = undef; # path to installed .pc files
my $aclocaldir = undef; # path to installed .m4 files

our $srcdir = undef;	# path to package source directory
our $objdir = undef;	# path to package build directory

our $debug = 0;		# enable debugging build
our $static = 0;	# enable static build (--disable-shared)

my $batch = 0;		# perform actions on dependencies as well as targets
our $toggle_batch = 0;  # reverse meaning of batch flag (default to batch mode)

my $njobs = 0;		# number of parallel jobs during makes (0 = autodetect)
my $supercompute = 0;   #  run unlimited parallel jobs during makes (yikes!)
my $ccache = 1;		# enable ccache support
my $force = 0;		# continue despite errors
my $show_env = 0;	# output environment variables this script changes
my $localconf = undef;	# local config overrides

my $pretend = 0;	# don't actually do anything
my $verbose = 0;	# enable verbose script output
my $quiet = 0;		# enable quiet script output
my $help = 0;
my $man = 0;

my $list_actions = 0;	# show the actions permitted by this script
my $list_targets = 0;	# show the targets known by this script

my $_run_app = undef;	# Which application did the user specify with -A?
our $run_app = undef;	# Which application does the user want to run?
my @run_paths = qw( bin sbin );

our $hostdist = 'autodetect';	# This host's distribution (e.g. gentoo).
our $buildspec = 'autodetect';  # This host's compiler specification.
our $hostspec = 'autodetect';	# The target host's compiler specification.
				# e.g. x86-pc-linux-gnu, arm-unknown-linux-gnu

# reset umask; fixes problems from mixing ccache with merge action
umask(0007) or die "unable to set umask to 0007: $!";

#######
# option processing helpers - use this API in configuration files

sub add_run_paths { push @run_paths, @_ }

#######
# process option arguments

sub usage { 
	warn @_, "\n" if @_;
	die <<USAGE;
usage: $app_name [<options>] <action>[+<action>...] [<targets>]
Build Options:
    -d|--debug		Enabled debugging in resulting builds
    -s|--static		Enabled static build (uses --disable-shared)

Advanced Build Options:
    -b|--build=...	Build libraries and programs on this platform.
    -t|--host=...	Build libraries and programs that run on this platform.
    -S|--batch		Perform actions on dependencies as well as targets.
    -j|--jobs=n		Set number of parallel jobs (default: $njobs)
    -J|--supercompute	Run an unlimited number of parallel jobs (for testing)
    -c|--ccache		Enable ccache support (is on by default; use --noccache)
    -f|--force		Continue building despite any errors along the way
    -E|--show-env	Show environment variables when we update them
    -l|--localconf	Config overrides
			(currently: $topdir/build.d/build.local)

Directory Options:
    -T|--topdir=...	Select location of svn repository. 
			(currently: $topdir)
    -C|--confdir=...	Select location of configuration directory.
			(currently: $topdir/build.d)
    -B|--builddir=...	Select location for build directories.
			(currently: $topdir/build)
    -I|--installdir=...	Select testing install directory (sets --prefix).
	                (currently: $topdir/install)
    -P|--prefix=...	Select "live" install prefix (overrides installdir)
       --destdir=...	Select destination install directory

Distribution Merge Options:
    -D|--distro=...	Sets package manager features (default: $hostdist)

Run Options:
    -A|--app=...	Select application to run (default: check build.local)

General Options:
    -p|--pretend	Do not actually perform actions
    -v|--verbose	Verbose output mode
    -q|--quiet		Quiet output mode
    -?|--help		Show built-in help
    --list-actions	Show permitted command actions
    --list-targets	Show permitted targets
USAGE
#	--man		Show built-in man page
}

my $result = GetOptions(
		"topdir|T=s" => \$topdir,

		"confdir|C=s" => \$confdir,
		"builddir|B=s" => \$builddir,
		"installdir|I=s" => \$installdir,
		"destdir=s" => \$destdir,
		"prefix|P=s" => \$prefix,
		"distro|D=s" => \$hostdist,
		"app|A=s" => \$_run_app,

		"debug|d!" => \$debug,
		"static|s!" => \$static,
		"batch|S!" => \$batch,

		"build|b=s" => \$buildspec,
		"host|t=s" => \$hostspec,

		"jobs|j=i" => \$njobs,
		"supercompute|J!" => \$supercompute,
		"ccache|c!" => \$ccache,
		"force|f!" => \$force,
		"show-env|E!" => \$show_env,
		"localconf|l=s" => \$localconf,

		"pretend|p!" => \$pretend,
		"verbose|v!" => \$verbose,
		"quiet|q!" => \$quiet,
		'help|?' => \$help, 
		'list-actions' => \$list_actions, 
		'list-targets' => \$list_targets, 
#		man => \$man,
	);
usage() if !$result || $help || $man;

pod2usage(2) unless $result;
pod2usage(1) if $help;
pod2usage(-exitstatus => 0, -verbose => 2) if $man;

# set-up configuration file paths
$confdir = $confdir || "$topdir/build.d";
my $conffile = "$confdir/build.conf";
die "'$confdir' is not a valid configuration directory.'" unless -f $conffile;

# set-up common options
$show_env = 1 if $pretend && $verbose;
$verbose = 1 if $pretend;
$verbose = 0 if $quiet;
$quiet = 0 if $pretend;

# guess number of jobs
# XXX: will not work for all platforms; needs revisiting, but okay for now
$njobs = int(`grep ^processor /proc/cpuinfo | wc -l`) unless $njobs;
$njobs = 1 unless $njobs;
$njobs = 'unlimited' if $supercompute;
print "+info: make will try to run $njobs parallel build jobs\n" if $verbose;

# extra arguments to pass to make
my @make_args = ();
push @make_args, '-k' if $force;
push @make_args, '-j' if $supercompute;
push @make_args, "-j$njobs" if int($njobs) > 1;

#######
# load package, dependency, and configure parameter definitions (see build.conf)

our $default_action;
our $default_target;

our @packages; 		# absolute package build order
our %dependencies; 	# package dependency lists
our %configure_params;	# package configure parameter tables

my @dist_actions = ( qw( packages package pkgclean merge purge ) );
our @actions = ( qw( bootstrap configure compile ),
		qw( install uninstall ),
		qw( clean dclean mclean ),
		qw( dist distcheck tarballs tarclean ),
		@dist_actions, 
		qw( check run allclean hostclean repoclean ),
	);

sub load_file_if_exists {
	my $file = shift;
	return 0 unless -f $file;
	do $file or die "error: unable to load '$file':\n$@";
	return 1;
}

# load primary definitions
load_file_if_exists($conffile);

sub set_configure_param {
	my ( $package, %params ) = @_;
	if (ref $package) {
		set_configure_param($_, %params) for @$package;
		return;
	}
	die "no package called '$package'" unless grep /^$package$/, @packages;
	for my $key ( keys %params ) {
		die "no parameter called '$key' in $package" 
			unless exists($configure_params{$package}->{$key});
		$configure_params{$package}->{$key} = $params{$key};
	}
	return 1;
}
sub set_global_configure_param {
	my ( %params ) = @_;
	for my $key ( keys %params ) {
		my $value = $params{$key};
		# reassign global variables for build system related params
		for ($key) {
		/^debug$/ and do { $debug = $value; last; };
		/^shared$/ and do { $static = !$value; last; };
		}
		set_configure_param($_, $key, $value) for @packages;
	}
	return 1;
}

$localconf = $localconf || "$confdir/build.local";
unless (-f $localconf) {
	# XXX: automatic upgrade code; remove this on or after May 1, 2006
	my $oldconf = "$topdir/build.local";
	if (-f $oldconf) {
		my $move_msg = "local configuration file to:\n" . 
			"\t$localconf\n\tfrom $oldconf\n";
		warn "Moving $move_msg";
		move($oldconf, $localconf) or
			die "error: unable to move $move_msg" .  "error: $!\n";
	} else {
	# XXX: end of automatic upgrade path (except closing else brace below)
	# Create 
	my $exampleconf = "$confdir/build.local.example";
	my $create_msg = "local configuration file:\n" . 
		"\t$localconf\n\tfrom $exampleconf\n";
	warn "Creating new $create_msg";
	copy($exampleconf, $localconf) or 
		die "error: unable to create $create_msg" . "error: $!\n";
	} # XXX
}
# allow overrides using the 'set*_configure_param' accessors
load_file_if_exists($localconf);

die list_actions() if $list_actions;
die list_targets() if $list_targets;

#######
# cross-compiling support

sub cross_compiling { return $buildspec ne $hostspec }
sub __detect { $_[0] eq 'autodetect' ? $_[1]->() : $_[0] }
sub autodetect_buildspec { __detect($buildspec, sub { 'x86-pc-linux-gnu' }) }
sub autodetect_hostspec { __detect($hostspec, sub { autodetect_buildspec() }) }

$buildspec = autodetect_buildspec();
$hostspec = autodetect_hostspec();
die "$hostspec-gcc not found." 
	if cross_compiling() && ! -x "/usr/bin/$hostspec-gcc";

# set-up paths
my $top_builddir = $builddir || "$topdir/build";
$builddir = "$top_builddir/$hostspec";
my $top_installdir = $installdir || "$topdir/install";
$installdir = "$top_installdir/$hostspec";

sub prefix_is_live { return $prefix }
sub prefix_path { prefix_is_live() || "$installdir/usr" }

$prefixdir = prefix_path();
$bindir = "$destdir$prefixdir/bin";

push @make_args, "DESTDIR=$destdir" if $destdir ne '';

#######
# process action and target arguments

my $action = shift @ARGV || $default_action;
die "error: No action specified! You must provide at least " .
	"one valid action.\n       You may combine actions with " .
	"'+', such as 'bootstrap+configure'.\n\n" . list_actions() .
	"\nFor more information, see '$app_name --help'.\n" unless $action;
my @action = split(/\+/, $action); 
for my $a ( @action ) {
	die "'$a' is not a valid action.\n" unless grep /^$a$/, @actions;
}

sub add_targets { 
	my @targets = map { ( add_targets(@{$dependencies{$_}}), $_) } @_;
	my %targets = map { $_ => 1 } @targets;
	return grep { delete $targets{$_} } @targets;
};
my @targets = @ARGV;
push @targets, $default_target if !@targets && $default_target;
for ( @targets ) {
	die "'$_' is not a valid target.\n" unless exists $configure_params{$_};
}
$batch = !$batch if $toggle_batch;
@targets = add_targets(@targets) if $batch;
die "error: no target specified!\nYou must specify at least " .
	"one valid target.\n\n" . list_targets() .
	"\nFor more information, see '$app_name --help'.\n" unless @targets;

print "$action: @targets\n" unless $quiet;

#######
# common action funtions

our $pkg;
my %actions;
my %act_deps;

sub easy_mkdir {
	my ( $path, $print ) = @_;
	eval { mkpath($path, $print, 0775) };
	die "unable to create $path:\n$@" if $@;
	return $path;
}
sub easy_chdir {
	my $path = shift;
	easy_mkdir($path) unless -d $path;
	print "+Changing to $path...\n" if $verbose;
	chdir($path) or die "unable to change to $path: $!";
}

sub create_working_paths {
	my @paths = ( 
			$builddir, $installdir,
			$bindir, $libdir, $includedir,
			$aclocaldir, $pkgconfigdir,
		);
	easy_mkdir($_, !$quiet) for @paths;
}

sub act {
	my $label = shift;
	print "Running $label in $pkg...", $pretend ? ' (dry run)' : '', "\n" 
		unless $quiet;
	print '+ ', join(' ', @_), "\n" if $verbose;
	return if $pretend;
	system(@_) == 0 or die "system @_ failed: $?";
}
sub callact {
	my $a = shift;
	die "no action '$a'" unless exists $actions{$a};

	$act_deps{$a}->() if exists $act_deps{$a};

	my $tgtdir = $a eq 'bootstrap' ? $srcdir : $objdir;
	if ($tgtdir ne $ENV{PWD}) {
		easy_chdir($tgtdir);
	}
	$actions{$a}->();
}

sub distfiles {
	return bsd_glob("$objdir/$pkg*.tar.gz");
}

sub _is_feature_param { return defined $_[0] && $_[0] =~ /^\d?$/; }
sub _feature_configure_params {
	my $spec = shift;
	my @keys = grep { _is_feature_param($spec->{$_}) } keys %$spec;
	my %spec = map { $_ => $spec->{$_} ? 'en' : 'dis' } @keys;
	return map { join('', '--', $spec{$_}, 'able-', $_) } @keys;
}
sub _is_package_param { return defined $_[0] && $_[0] !~ /^\d?$/; }
sub _package_configure_params {
	my $spec = shift;
	my @keys = grep { _is_package_param($spec->{$_}) } keys %$spec;
	my %spec = map { $_ => $spec->{$_} ? '' : 'out' } @keys;
	return map { join('', '--', 'with', $spec{$_}, '-', $_, 
			$spec->{$_} ? '=' : '', $spec->{$_} ) } @keys;
}
sub configure_params {
	my $spec = $configure_params{$pkg};
	my @spec = ( 
			"--srcdir=$srcdir", 
			"--prefix=$prefixdir",
			_feature_configure_params($spec),
			_package_configure_params($spec),
		);
	unshift @spec, "--build=$buildspec", "--host=$hostspec"
		if cross_compiling();
	# XXX: the following changes require further configure.ac changes;
	#      however, they should not interfere until that time
	my $deps = $dependencies{$pkg};
	my %deps = map { $_ => /^lib(.*)$/ } @$deps;
	push @spec, map { "--with-" . $deps{$_} . "=$topdir/$_" } @$deps;
	return @spec;
}

sub list_files {
	my $label = shift;
	print $label, join(", ", map { basename($_) } @_), "\n";
	return @_;
}
sub remove_files {
	my ( $label, @files ) = @_;
	for my $p ( @files ) {
		print "+removing $pkg $label: $p\n";
		unlink($p) or die "can't unlink $p: $!"; 
	}
}

#######
# autodetection helpers

sub autodetect_probe_path {
	my ( $type, $setting ) = @_;
	return $setting unless $setting eq 'autodetect';
	# probes can factor multiple scores, allow configurations to be 
	#  selected by more than one criteria.
	my %scores = ( );
	for my $dir ( bsd_glob("$confdir/$type/*") ) {
		my $file = "$dir/detect.pl";
		next unless load_file_if_exists($file);
		no strict 'refs';
		my $f = $type . '_detect';
		$scores{basename($dir)} = eval "$f()";
	}
	# pick the top scoring probe
	( $setting ) = sort { $scores{$a} <=> $scores{$b} } keys %scores;
	return $setting;
} 
sub autodetect_probe {
	my ( $type, $setting ) = @_;
	( $setting ) = autodetect_probe_path($type, $setting);
	my $xconfdir = "$confdir/$type/$setting";
	my $conf = "$xconfdir/$type.pl";
	if (load_file_if_exists($conf)) {
		print "+Using '$type' functions '$setting'\n:" .
			"\t$conf\n" if $verbose;
		load_file_if_exists("$xconfdir/$type.conf");
		load_file_if_exists("$xconfdir/$type.local");
	} else {
		warn <<NOCODE unless $quiet;
warning: The '$type.pl' script does not exist for '$setting'.
warning: '$type' actions will be disabled until one is created!
NOCODE
	} 
	return $setting;
}

#########
# Generic callback support

my %callbacks;

sub get_extended_actions { map { %$_ } values %callbacks }

sub set_callbacks {
	my ( $type, %newfuncs ) = @_;
	$callbacks{$type} = {} unless exists $callbacks{$type};
	$callbacks{$type}->{$_} = $newfuncs{$_} for keys %newfuncs;
	return 1;
}
sub set_callback { $callbacks{$_[0]}->{$_[1]} = $_[2] }
sub set_default_callback {
	my ( $type, $callback, $func ) = @_;
	return unless exists $callbacks{$type};
	return if exists $callbacks{$type}->{$callback};
	return set_callback($type, $callback, $func);
}
sub run_callback { $callbacks{$_[0]}->{$_[1]}->(@_) }
 
 
########
# Target/host distribution support

sub set_dist_callbacks { set_callbacks('dist', @_) }
sub set_dist_detect { set_callback(qw( dist detect ), $_[0]) }
sub dist_detect { run_callback(qw( dist detect )) }

# probe the given hostdist; load its configuration files
$hostdist = autodetect_probe('dist', $hostdist);

# create standard dist accessor implementations
set_default_callback(qw( dist packages ), sub { 
		list_files("$hostdist: $pkg packages: ", dist_pkgfiles()) 
	});
set_default_callback(qw( dist pkgclean ), sub { 
		remove_files("$hostdist package", dist_pkgfiles()) 
	});

# provide debuggable defaults and automatic accessors
for my $f ( @dist_actions, qw( pkgfiles ) ) {
	set_default_callback('dist', $f, sub {
			die "+BUG: unable to $f packages under '$hostdist'\n"
		});
	no strict 'refs';
	my $callback = "dist_$f";
	*$callback = sub { return run_callback('dist', $f, @_) };
}

######
#  Common action functions

sub run_app_path {
	die "error: No application was specified in build.local or with -A.\n"
		unless $run_app;
	my $rootpath = $destdir . $prefixdir;
	for my $path ( @run_paths ) {
		my $target = "$rootpath/$path/$run_app";
		print "+checking for $target..." if $verbose;
		next unless -x $target;
		print "+found $target..." unless $quiet;
		return $target;
	}
	return $rootpath . $run_app;
	
}

%actions = (
	get_extended_actions(),
	bootstrap => sub { act('bootstrap', './bootstrap') },
	configure => sub { 
		act('configure', "$srcdir/configure", configure_params()); 
	},
	compile => sub { act('compile', 'make', @make_args); },
	check => sub { act('check', 'make', @make_args, 'check'); },
	install => sub { act('install', 'make', @make_args, 'install'); },
	uninstall => sub { act('uninstall', 'make', @make_args, 'uninstall'); },
	run => sub {
		 # XXX: This needs to be generalized, but it works for now.
		 #  For what it's worth, it is "equivalent" to the old .sh
		return unless $pkg eq 'minisip';
		$ENV{LD_LIBRARY_PATH} = join(':', ( map { "$builddir/$_/.libs" } @packages ), $libdir);
		act('run', run_app_path());
	},
	tarballs => sub { list_files("$pkg tarballs: ", distfiles()) },
	tarclean => sub { remove_files("tarballs", distfiles()); },
	dist => sub { act('distribution', 'make', @make_args, 'dist'); },
	distcheck => sub { act('distcheck', 'make', @make_args, 'distcheck'); },
	clean => sub { act('cleanup', 'make', 'clean'); }, 
	dclean => sub { act('distribution cleanup', 'make', 'distclean'); },
	mclean => sub { act('developer cleanup', 'make', 'maintainer-clean'); },
	allclean => sub {
		return unless -e "$objdir/Makefile";
		callact('uninstall');
		callact('mclean');
	},
	hostclean => sub { 
		#  remove but recreate objdir, in case of pending actions
		easy_chdir($builddir);
		rmtree($objdir, 1, 0);
		easy_chdir($objdir);
	},
	repoclean => sub { },
);

# common checks for preconditions
my $need_bootstrap = sub { callact('bootstrap') unless -e "$srcdir/configure" };
my $need_configure = sub { callact('configure') unless -e "$objdir/Makefile" };
my $need_compile = sub { callact('compile') }; # always rebuild, just in case
my $need_install = sub { 
	callact('install') unless -x run_app_path();
}; 
my $need_tarclean = sub { callact('tarclean'); $need_compile->() };
my $need_dist = sub { callact('dist') unless scalar(distfiles()) };
my $need_package = sub { callact('package') unless scalar(dist_pkgfiles()) };
my $need_allclean = sub { callact('allclean') };

%act_deps = (
	configure => $need_bootstrap,
	compile => $need_configure,
	check => $need_compile,
	install => $need_compile,
	run => $need_install,
	dist => $need_tarclean,
	distcheck => $need_tarclean,
	'package' => $need_dist,
	'packages' => $need_package,
	merge => $need_package,
	clean => $need_configure,
	dclean => $need_configure,
	mclean => $need_configure,
	hostclean => $need_allclean,
	repoclean => $need_allclean,
);

sub list_actions {
	return "This script supports the following actions:\n" .
		wrap("\t", "\t", join(", ", @actions)) . "\n";
}

sub list_targets {
	return "This script knows about the following targets:\n" .
		wrap("\t", "\t", join(", ", @packages)) . "\n";
}

########################################################################
#  Main Program Start

# override application to run, if user specified one via CLI
$run_app = $_run_app if defined $_run_app;

if ($verbose) {
	print "+Top directory:              $topdir\n";
	print "+Build directory:            $builddir\n";
	print "+Install \$DESTDIR directory: $destdir\n" if $destdir;
	print "+Install --prefix directory: $prefixdir\n";
	print "+Full path to local install: $destdir$prefixdir\n" if $destdir;
}

# setup common environment
#  LDFLAGS: add local install library path
$ENV{LDFLAGS} ||= '';
$libdir = "$destdir$prefixdir/lib";
$ENV{LDFLAGS} .= " -L$libdir";
#  CPPFLAGS: add local install include path (XXX: needed?)
$ENV{CPPFLAGS} ||= '';
$includedir = "$destdir$prefixdir/include";
$ENV{CPPFLAGS} .= " -I$includedir ";
#  CXXFLAGS: enabled all warnings and (optionally) debugging
$ENV{CXXFLAGS} ||= '';
$ENV{CXXFLAGS} .= " -Wall";
$ENV{CXXFLAGS} .= " -ggdb" if $debug;

$aclocaldir = "$destdir$prefixdir/share/aclocal";

sub aclocal_flags {
	my @m4_paths = map { "$topdir/$_/m4" } @packages;
	my @aclocal_flags = map { ( '-I', $_ ) } @m4_paths, $aclocaldir;
	return join(' ', '', @aclocal_flags);
}
$ENV{ACLOCAL_FLAGS} ||= '';
$ENV{ACLOCAL_FLAGS} .= aclocal_flags();

# pkg-config search order (tries to find the "most recent copy"):
#   1) Project directories
#   2) Local install directory
#   3) System directories 
$pkgconfigdir = "$destdir$prefixdir/lib/pkgconfig";
my @pkgconfigdirs = ( ( map { "$builddir/$_" } @packages ), $pkgconfigdir );
push @pkgconfigdirs, $ENV{PKG_CONFIG_PATH} if $ENV{PKG_CONFIG_PATH};
$ENV{PKG_CONFIG_PATH} = join(':', @pkgconfigdirs);

if ($ccache) {
	$ENV{PATH} = join(':', '/usr/lib/ccache/bin', $ENV{PATH});
	$ENV{CCACHE_DIR} = easy_mkdir(File::Spec->catdir($topdir, '.ccache'))
		unless $ENV{CCACHE_DIR} && -d $ENV{CCACHE_DIR};
}

# special pre-target processing
for ( @action ) {
/^repoclean$/ and do {
	# repoclean always operates on all packages
	@targets = @packages;
	last
};
# no pre-target work to do
}

create_working_paths();

for $pkg ( @targets ) {
	# XXX: be afraid! dynamic scoping... icky icky icky, but so darn handy
	local $pkg = $pkg;
	local $srcdir = File::Spec->catdir($topdir, $pkg);
	local $objdir = File::Spec->catdir($builddir, $pkg);
	easy_mkdir($objdir);

	print "+Source directory: $srcdir\n",
		"+Object directory: $objdir\n" if $verbose;

	if ($show_env) {
		my @envvars = ( qw( 
				PWD PATH CCACHE_DIR PKG_CONFIG_PATH
				CPPFLAGS CXXFLAGS LDFLAGS 
			) );
		my @env = map { "\n\t$_=" . $ENV{$_} } @envvars;
		print "Build environment for $pkg: @env\n";
	}

	callact($_) for @action;
}

# special post-target processing
for ( @action ) {
/^hostclean$/ and do {
	easy_chdir($topdir);
	rmtree( [ $builddir, $installdir ], 1, 0 );
	exit(0);
};
/^repoclean$/ and do {
	easy_chdir($topdir);
	rmtree( [ $top_builddir, $top_installdir ], 1, 0 );
	exit(0);
};
# no post-target work to do
}
