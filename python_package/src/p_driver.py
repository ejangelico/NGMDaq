import SIS3316

if __name__ == "__main__":
    sis = SIS3316.SIS3316()
    sis.configure()
    sis.initialize()

    # Thsi is really "acquire data"
    sis.startAcquisition(doLoop=False, n_hours=10.0)

    # Then what?
    #### SystemMT streaming reading - but no Python interface
