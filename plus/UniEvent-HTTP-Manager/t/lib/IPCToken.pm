package IPCToken;
use 5.016;
use warnings;
use IPC::SysV qw(IPC_CREAT IPC_RMID IPC_PRIVATE);

sub new {
    my ($class, $initial) = @_;
    my $id = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT)
        || die("semget failed:: $!");

    my $obj = bless {id => $id} => $class;
    $obj->inc($initial) if $initial;
    return $obj;
}

sub DESTROY {
    my $self = shift;
    my $id = $self->{id};
    shmctl($id, IPC_RMID, 0) || die "shmctl: $!";
}

sub inc {
    my ($self, $val) = @_;
    $val //= 1;
    my $opstring = pack("s!s!s!", 0, $val, 0);
    my $id = $self->{id};
    semop($id, $opstring) || die "semop ($$): $!";
}

sub dec {
    my ($self, $val) = @_;
    ($val //= 1) *= -1;
    my $opstring = pack("s!s!s!", 0, $val, 0);
    my $id = $self->{id};
    semop($id, $opstring) || die "semop ($$): $!";
}

sub await {
    my ($self, $timeout) = @_;
    eval {
         local $SIG{ALRM} = sub { die "alarm\n" };
         alarm $timeout;

         my $opstring = pack("s!s!s!", 0, 0, 0);
         my $id = $self->{id};
         semop($id, $opstring) || die "semop ($$): $!";

         alarm 0;
     };
     if ($@) {
        if ($@ eq "alarm\n") {
            return "timed out";
        }
        else {
            return @$;
        }
     }
     else {
        return undef;
     }
}



1;
