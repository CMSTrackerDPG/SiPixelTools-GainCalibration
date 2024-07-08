#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
import mplhep as hep


def BGV(d):
    return -0.13 * np.exp(-3.13 * 10 ** (-2) * d) + 1.38


def BGV_inv(bgv):
    # print(bgv)
    # print((1.38 - bgv) / 0.13)
    return np.log((1.38 - bgv) / 0.13) / (-3.13 * 10 ** (-2))


if __name__ == "__main__":
    corrs_bpix_2023_v0 = {1: 1.037, 2: 1.031, 3: 1.020, 4: 1.012}
    corrs_fpix_2023_v0 = {1: 1.14, 2: 1.14, 3: 1.14}

    lumi_2017 = 49.97
    lumi_2018 = 65.77
    lumi_2023_v0 = 41.97
    lumi_2023_v1 = 62.57  # in fb^{-1}
    lumi_2023_v3 = lumi_2023_v1 + 10.843993752  # in fb^{-1}
    lumi_2024_v0 = lumi_2023_v3 +  0.000401619  # in fb^{-1}
    lumi_2024_v1 = lumi_2024_v0 +  29.214249009  # in fb^{-1}

    print(f"Luminosity until end 2018: {lumi_2017 + lumi_2018:>7.2f} [1/fb]")
    print(f"Luminosity until 2023_v0:  {lumi_2017 + lumi_2018 + lumi_2023_v0:>7.2f} [1/fb]")
    print(f"Luminosity until 2023_v1:  {lumi_2017 + lumi_2018 + lumi_2023_v1:>7.2f} [1/fb]\n")
    print(f"Luminosity until 2023_v3:  {lumi_2017 + lumi_2018 + lumi_2023_v3:>7.2f} [1/fb]\n")
    print(f"Luminosity until 2024_v0:  {lumi_2017 + lumi_2018 + lumi_2024_v0:>7.2f} [1/fb]\n")
    print(f"Luminosity until 2024_v1:  {lumi_2017 + lumi_2018 + lumi_2024_v1:>7.2f} [1/fb]\n")

    # doses in Mrad
    fpix_factor_ring1 = 0.212
    fpix_factor_ring2 = 0.056

    dose_2023_v0_layer1 = 0.35 * lumi_2023_v0
    dose_2023_v0_layer2 = 0.0717 * (lumi_2017 + lumi_2018 + lumi_2023_v0)
    dose_2023_v0_layer3 = 0.0438 * (lumi_2017 + lumi_2018 + lumi_2023_v0)
    dose_2023_v0_layer4 = 0.0236 * (lumi_2017 + lumi_2018 + lumi_2023_v0)
    dose_2023_v0_ring1 = fpix_factor_ring1 * (lumi_2017 + lumi_2018 + lumi_2023_v0)
    dose_2023_v0_ring2 = fpix_factor_ring2 * (lumi_2017 + lumi_2018 + lumi_2023_v0)

    dose_2023_v1_layer1 = 0.35 * lumi_2023_v1
    dose_2023_v1_layer2 = 0.0717 * (lumi_2017 + lumi_2018 + lumi_2023_v1)
    dose_2023_v1_layer3 = 0.0438 * (lumi_2017 + lumi_2018 + lumi_2023_v1)
    dose_2023_v1_layer4 = 0.0236 * (lumi_2017 + lumi_2018 + lumi_2023_v1)
    dose_2023_v1_ring1 = fpix_factor_ring1 * (lumi_2017 + lumi_2018 + lumi_2023_v1)
    dose_2023_v1_ring2 = fpix_factor_ring2 * (lumi_2017 + lumi_2018 + lumi_2023_v1)

    dose_2023_v3_layer1 = 0.35 * lumi_2023_v3
    dose_2023_v3_layer2 = 0.0717 * (lumi_2017 + lumi_2018 + lumi_2023_v3)
    dose_2023_v3_layer3 = 0.0438 * (lumi_2017 + lumi_2018 + lumi_2023_v3)
    dose_2023_v3_layer4 = 0.0236 * (lumi_2017 + lumi_2018 + lumi_2023_v3)
    dose_2023_v3_ring1 = fpix_factor_ring1 * (lumi_2017 + lumi_2018 + lumi_2023_v3)
    dose_2023_v3_ring2 = fpix_factor_ring2 * (lumi_2017 + lumi_2018 + lumi_2023_v3)

    dose_2024_v0_layer1 = 0.35 * lumi_2024_v0
    dose_2024_v0_layer2 = 0.0717 * (lumi_2017 + lumi_2018 + lumi_2024_v0)
    dose_2024_v0_layer3 = 0.0438 * (lumi_2017 + lumi_2018 + lumi_2024_v0)
    dose_2024_v0_layer4 = 0.0236 * (lumi_2017 + lumi_2018 + lumi_2024_v0)
    dose_2024_v0_ring1 = fpix_factor_ring1 * (lumi_2017 + lumi_2018 + lumi_2024_v0)
    dose_2024_v0_ring2 = fpix_factor_ring2 * (lumi_2017 + lumi_2018 + lumi_2024_v0)

    dose_2024_v1_layer1 = 0.35 * lumi_2024_v1
    dose_2024_v1_layer2 = 0.0717 * (lumi_2017 + lumi_2018 + lumi_2024_v1)
    dose_2024_v1_layer3 = 0.0438 * (lumi_2017 + lumi_2018 + lumi_2024_v1)
    dose_2024_v1_layer4 = 0.0236 * (lumi_2017 + lumi_2018 + lumi_2024_v1)
    dose_2024_v1_ring1 = fpix_factor_ring1 * (lumi_2017 + lumi_2018 + lumi_2024_v1)
    dose_2024_v1_ring2 = fpix_factor_ring2 * (lumi_2017 + lumi_2018 + lumi_2024_v1)

    corr_2023_v0_ring1 = BGV(dose_2023_v0_ring1) / BGV(0)
    corr_2023_v0_ring2 = BGV(dose_2023_v0_ring2) / BGV(0)

    corr_2023_v1_ring1 = BGV(dose_2023_v1_ring1) / BGV(0)
    corr_2023_v1_ring2 = BGV(dose_2023_v1_ring2) / BGV(0)

    corr_2023_v3_ring1 = BGV(dose_2023_v3_ring1) / BGV(0)
    corr_2023_v3_ring2 = BGV(dose_2023_v3_ring2) / BGV(0)

    corr_2024_v0_ring1 = BGV(dose_2024_v0_ring1) / BGV(0)
    corr_2024_v0_ring2 = BGV(dose_2024_v0_ring2) / BGV(0)
    
    corr_2024_v1_ring1 = BGV(dose_2024_v1_ring1) / BGV(0)
    corr_2024_v1_ring2 = BGV(dose_2024_v1_ring2) / BGV(0)

    avg_corr_2023_v0_fpix = round((corr_2023_v0_ring1 + corr_2023_v0_ring2) / 2 * (49 / 47) , 3)
    avg_corr_2023_v1_fpix = round((corr_2023_v1_ring1 + corr_2023_v1_ring2) / 2 * (49 / 47) , 3)
    avg_corr_2023_v3_fpix = round((corr_2023_v3_ring1 + corr_2023_v3_ring2) / 2 * (49 / 47) , 3)
    avg_corr_2024_v0_fpix = round((corr_2024_v0_ring1 + corr_2024_v0_ring2) / 2 * (49 / 47) , 3)
    avg_corr_2024_v1_fpix = round((corr_2024_v1_ring1 + corr_2024_v1_ring2) / 2 * (49 / 47) , 3)


    print("BPix Doses:")
    # print(f"dose in Run3 until 2023_v0: {dose_2023_v0:.4f} Mrad")
    print(f"dose in Run3 until 2023_v0 for layer 1: {dose_2023_v0_layer1:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v0 for layer 2: {dose_2023_v0_layer2:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v0 for layer 3: {dose_2023_v0_layer3:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v0 for layer 4: {dose_2023_v0_layer4:>7.4f} Mrad\n")

    print(f"dose in Run3 until 2023_v1 for layer 1: {dose_2023_v1_layer1:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v1 for layer 2: {dose_2023_v1_layer2:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v1 for layer 3: {dose_2023_v1_layer3:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v1 for layer 4: {dose_2023_v1_layer4:>7.4f} Mrad\n\n")

    print(f"dose in Run3 until 2023_v3 for layer 1: {dose_2023_v3_layer1:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v3 for layer 2: {dose_2023_v3_layer2:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v3 for layer 3: {dose_2023_v3_layer3:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v3 for layer 4: {dose_2023_v3_layer4:>7.4f} Mrad\n\n")

    print(f"dose in Run3 until 2024_v0 for layer 1: {dose_2024_v0_layer1:>7.4f} Mrad")
    print(f"dose in Run3 until 2024_v0 for layer 2: {dose_2024_v0_layer2:>7.4f} Mrad")
    print(f"dose in Run3 until 2024_v0 for layer 3: {dose_2024_v0_layer3:>7.4f} Mrad")
    print(f"dose in Run3 until 2024_v0 for layer 4: {dose_2024_v0_layer4:>7.4f} Mrad\n\n")

    print(f"dose in Run3 until 2024_v1 for layer 1: {dose_2024_v1_layer1:>7.4f} Mrad")
    print(f"dose in Run3 until 2024_v1 for layer 2: {dose_2024_v1_layer2:>7.4f} Mrad")
    print(f"dose in Run3 until 2024_v1 for layer 3: {dose_2024_v1_layer3:>7.4f} Mrad")
    print(f"dose in Run3 until 2024_v1 for layer 4: {dose_2024_v1_layer4:>7.4f} Mrad\n\n")

    # fpix_slope_correction_to_bpix = 47 / 49
    # fpix_dose_2023_v0 = BGV_inv(
    #     fpix_slope_correction_to_bpix * corrs_fpix_2023_v0[1] * BGV(0)
    # )
    # fpix_dose_factor = fpix_dose_2023_v0 / (lumi_2017 + lumi_2018 + lumi_2023_v0)
    # fpix_dose_2023_v1 = fpix_dose_factor * (lumi_2017 + lumi_2018 + lumi_2023_v1)

    # print(f"fpix dose for 2023_v0: {fpix_dose_2023_v0:>7.4f} Mrad")
    # print(f"fpix factor for 2023_v0: {fpix_dose_factor:>7.4f} Mrad*fb")

    # print("Daneks fpix numbers")
    # ring_1_dose = 34.5
    # ring_2_dose = 9.1
    print("FPix Doses:")
    print(f"dose in Run3 until 2023_v0 for ring 1: {dose_2023_v0_ring1:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v0 for ring 2: {dose_2023_v0_ring2:>7.4f} Mrad\n")
    print(f"dose in Run3 until 2023_v1 for ring 1: {dose_2023_v1_ring1:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v1 for ring 2: {dose_2023_v1_ring2:>7.4f} Mrad\n")
    print(f"dose in Run3 until 2023_v3 for ring 1: {dose_2023_v3_ring1:>7.4f} Mrad")
    print(f"dose in Run3 until 2023_v3 for ring 2: {dose_2023_v3_ring2:>7.4f} Mrad\n\n")
    print(f"dose in Run3 until 2024_v0 for ring 1: {dose_2024_v0_ring1:>7.4f} Mrad")
    print(f"dose in Run3 until 2024_v0 for ring 2: {dose_2024_v0_ring2:>7.4f} Mrad\n\n")
    print(f"dose in Run3 until 2024_v1 for ring 1: {dose_2024_v1_ring1:>7.4f} Mrad")
    print(f"dose in Run3 until 2024_v1 for ring 2: {dose_2024_v1_ring2:>7.4f} Mrad\n\n")
    
    print(f"corr in Run3 until 2023_v1 for ring 1:        {corr_2023_v1_ring1:.4f}")
    print(f"corr in Run3 until 2023_v1 for ring 2:        {corr_2023_v1_ring2:.4f}\n")
    print(f"corr in Run3 until 2023_v3 for ring 1:        {corr_2023_v3_ring1:.4f}")
    print(f"corr in Run3 until 2023_v3 for ring 2:        {corr_2023_v3_ring2:.4f}\n")
    print(f"corr in Run3 until 2024_v0 for ring 1:        {corr_2024_v0_ring1:.4f}")
    print(f"corr in Run3 until 2024_v0 for ring 2:        {corr_2024_v0_ring2:.4f}\n")
    print(f"corr in Run3 until 2024_v1 for ring 1:        {corr_2024_v1_ring1:.4f}")
    print(f"corr in Run3 until 2024_v1 for ring 2:        {corr_2024_v1_ring2:.4f}\n")

    print(f"corr in Run3 until 2023_v0 for ring 1 x49/47: {corr_2023_v0_ring1 * (49 / 47):.4f}")
    print(f"corr in Run3 until 2023_v0 for ring 2 x49/47: {corr_2023_v0_ring2 * (49 / 47):.4f}\n")
    print(f"corr in Run3 until 2023_v1 for ring 1 x49/47: {corr_2023_v1_ring1 * (49 / 47):.4f}")
    print(f"corr in Run3 until 2023_v1 for ring 2 x49/47: {corr_2023_v1_ring2 * (49 / 47):.4f}\n")
    print(f"corr in Run3 until 2023_v3 for ring 1 x49/47: {corr_2023_v3_ring1 * (49 / 47):.4f}")
    print(f"corr in Run3 until 2023_v3 for ring 2 x49/47: {corr_2023_v3_ring2 * (49 / 47):.4f}\n")
    print(f"corr in Run3 until 2024_v0 for ring 1 x49/47: {corr_2024_v0_ring1 * (49 / 47):.4f}")
    print(f"corr in Run3 until 2024_v0 for ring 2 x49/47: {corr_2024_v0_ring2 * (49 / 47):.4f}\n")
    print(f"corr in Run3 until 2024_v1 for ring 1 x49/47: {corr_2024_v1_ring1 * (49 / 47):.4f}")
    print(f"corr in Run3 until 2024_v1 for ring 2 x49/47: {corr_2024_v1_ring2 * (49 / 47):.4f}\n")
    print(f"avg of ring1 and ring2 corr in Run3 until 2023_v0 x49/47: {avg_corr_2023_v0_fpix:.4f}")
    print(f"avg of ring1 and ring2 corr in Run3 until 2023_v1 x49/47: {avg_corr_2023_v1_fpix:.4f}\n")
    print(f"avg of ring1 and ring2 corr in Run3 until 2024_v0 x49/47: {avg_corr_2024_v0_fpix:.4f}\n")
    print(f"avg of ring1 and ring2 corr in Run3 until 2024_v1 x49/47: {avg_corr_2024_v1_fpix:.4f}\n")

    # factor = BGV(dose_2023_v1) / BGV(dose_2023_v0)
    # print(f"BGV adjustment factor from v1 to v2: {factor}")

    corrs_bpix_2023_v0_new_model = {
        i: round(BGV(dose) / BGV(0), 3)
        for i, dose in zip(
            range(1, 5),
            [
                dose_2023_v0_layer1,
                dose_2023_v0_layer2,
                dose_2023_v0_layer3,
                dose_2023_v0_layer4,
            ],
        )
    }
    corrs_bpix_2023_v1 = {
        i: round(BGV(dose) / BGV(0), 3)
        for i, dose in zip(
            range(1, 5),
            [
                dose_2023_v1_layer1,
                dose_2023_v1_layer2,
                dose_2023_v1_layer3,
                dose_2023_v1_layer4,
            ],
        )
    }
    corrs_bpix_2023_v3 = {
        i: round(BGV(dose) / BGV(0), 3)
        for i, dose in zip(
            range(1, 5),
            [
                dose_2023_v3_layer1,
                dose_2023_v3_layer2,
                dose_2023_v3_layer3,
                dose_2023_v3_layer4,
            ],
        )
    }
    corrs_bpix_2024_v0 = {
        i: round(BGV(dose) / BGV(0), 3)
        for i, dose in zip(
            range(1, 5),
            [
                dose_2024_v0_layer1,
                dose_2024_v0_layer2,
                dose_2024_v0_layer3,
                dose_2024_v0_layer4,
            ],
        )
    }
    corrs_bpix_2024_v1 = {
        i: round(BGV(dose) / BGV(0), 3)
        for i, dose in zip(
            range(1, 5),
            [
                dose_2024_v1_layer1,
                dose_2024_v1_layer2,
                dose_2024_v1_layer3,
                dose_2024_v1_layer4,
            ],
        )
    }
    corrs_fpix_2023_v0_new_model = {
        1 : avg_corr_2023_v0_fpix,
        2 : avg_corr_2023_v0_fpix,
        3 : avg_corr_2023_v0_fpix,
    }
    corrs_fpix_2023_v1 = {
        1 : avg_corr_2023_v1_fpix,
        2 : avg_corr_2023_v1_fpix,
        3 : avg_corr_2023_v1_fpix,
    }
    corrs_fpix_2023_v3 = {
        1 : round(corr_2023_v3_ring1 * (49 / 47), 4),
        2 : round(corr_2023_v3_ring2 * (49 / 47), 4),
    }
    corrs_fpix_2024_v0 = {
        1 : round(corr_2024_v0_ring1 * (49 / 47), 4),
        2 : round(corr_2024_v0_ring2 * (49 / 47), 4),
    }
    corrs_fpix_2024_v1 = {
        1 : round(corr_2024_v1_ring1 * (49 / 47), 4),
        2 : round(corr_2024_v1_ring2 * (49 / 47), 4),
    }

    print("values used for 2023_v0 (old model fpix):")
    print(f"corrs_bpix_2023_v0 = {corrs_bpix_2023_v0}")
    print(f"corrs_fpix_2023_v0 = {corrs_fpix_2023_v0}\n")
    print("values for 2023_v0 new model:")
    print(f"corrs_bpix_2023_v0 = {corrs_bpix_2023_v0_new_model}")
    print(f"corrs_fpix_2023_v0 = {corrs_fpix_2023_v0_new_model}\n")
    print("values used for 2023_v1 new model:")
    print(f"corrs_bpix_2023_v1 = {corrs_bpix_2023_v1}")
    print(f"corrs_fpix_2023_v1 = {corrs_fpix_2023_v1}\n")
    print("values used for 2023_v3 new model:")
    print(f"corrs_bpix_2023_v3 = {corrs_bpix_2023_v3}")
    print(f"corrs_fpix_2023_v3 = {corrs_fpix_2023_v3}\n")
    print("values used for 2024_v0 new model:")
    print(f"corrs_bpix_2024_v0 = {corrs_bpix_2024_v0}")
    print(f"corrs_fpix_2024_v0 = {corrs_fpix_2024_v0}\n")
    print("values used for 2024_v1 new model:")
    print(f"corrs_bpix_2024_v1 = {corrs_bpix_2024_v1}")
    print(f"corrs_fpix_2024_v1 = {corrs_fpix_2024_v1}\n")

    # plt.rcParams.update({
    #     "text.usetex": True,
    #     "font.family": "sans-serif",
    #     "font.sans-serif": "Helvetica",
    # })
    hep.style.use("CMS")
    px = 1/plt.rcParams['figure.dpi']  # pixel in inches
    fig, ax = plt.subplots(figsize=(4*400*px, 3*400*px))
    labels = [
        r'$-0.13V * e^{-0.0313Mrad^{-1} * dose} + 1.38V$',
        r'$1. + dose * 0.0045 Mrad^{-1}$',
    ]

    y_upper_lim = 1.2
    x_upper_lim = 70
    x = np.linspace(0, x_upper_lim, 100)
    y = BGV(x) / BGV(0)
    y_old = 1 + x * 0.0045

    new_model = ax.plot(x, y, linewidth=2.0, label=labels[0], linestyle='solid')
    old_model = ax.plot(x, y_old, linewidth=2.0, label=labels[1], linestyle='dashdot')

    ax.set(xlim=(0, x_upper_lim), xticks=np.arange(0, x_upper_lim, 10),
           ylim=(1.0, y_upper_lim), yticks=np.arange(1.0, y_upper_lim, 0.05))
    ax.set_xlabel('dose /(Mrad)', loc='right')
    ax.set_ylabel('slope correction factor', loc='top')
    ax.grid(True, axis='both')

    ax.legend()

    vlines = [
        [dose_2024_v1_layer1, "layer 1"],
        [dose_2024_v1_layer2, "layer 2"],
        [dose_2024_v1_layer3, "layer 3"],
        [dose_2024_v1_layer4, "layer 4"],
        [dose_2024_v1_ring1, "ring 1"],
        [dose_2024_v1_ring2, "ring 2"],
    ]
    for vline in vlines:
        ax.axvline(x=vline[0], color='red', linestyle='--')
        ax.text(vline[0]-1, y_upper_lim + 0.005, vline[1], rotation=90, size='x-small')
    ax.text(x_upper_lim, y_upper_lim + 0.005, "Gain Calibration 2024_v1", horizontalalignment='right')

    plt.savefig("calc_corrections_2024_v1.png")
    plt.show()
    # input("Enter")
