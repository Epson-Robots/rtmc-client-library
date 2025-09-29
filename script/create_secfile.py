#!/usr/bin/env python3

"""
This is a module for generating secure file
"""

#  Copyright 2025 Seiko Epson Corporation

#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at

#       http://www.apache.org/licenses/LICENSE-2.0

#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import os
import subprocess
import shutil
import xml.etree.ElementTree as ET
import xml.dom.minidom

# The foleder name where security files are stored
CA_CERT_FOLDER="ca_cert"
CLIENT_CERT_FOLDER="client_cert"
RC_CERT_FOLDER="rc_cert"


# File names for the governance document and the permissions document
GOVERNANCE_FILE="governance_example.xml"
CLIENT_PERMISSIONS_FILE="client_permissions_example.xml"
RC_PERMISSIONS_FILE="rc_permissions_example.xml"

# File names for the generated private key and certificate
CA_CERT_KEY_FILE=f"{CA_CERT_FOLDER}/ca_private_key_example.pem"
CA_CERT_FILE=f"{CA_CERT_FOLDER}/ca_cert_example.pem"
CLIENT_CERT_KEY_FILE=f"{CLIENT_CERT_FOLDER}/client_private_key_example.pem"
CLIENT_CSR_FILE=f"{CLIENT_CERT_FOLDER}/client_cert_example.csr"
CLIENT_CERT_FILE=f"{CLIENT_CERT_FOLDER}/client_cert_example.pem"
RC_CERT_KEY_FILE=f"{RC_CERT_FOLDER}/rc_private_key_example.pem"
RC_CSR_FILE=f"{RC_CERT_FOLDER}/rc_cert_example.csr"
RC_CERT_FILE=f"{RC_CERT_FOLDER}/rc_cert_example.pem"

# File name of the signed governance document
CLIENT_GOVERNANCE_SMIME=f"{CLIENT_CERT_FOLDER}/client_governance_example.smime"
RC_GOVERNANCE_SMIME=f"{RC_CERT_FOLDER}/rc_governance_example.smime"

# File name of the signed permissions document
CLIENT_PERMISSIONS_SMIME=f"{CLIENT_CERT_FOLDER}/client_permissions_example.smime"
RC_PERMISSIONS_SMIME=f"{RC_CERT_FOLDER}/rc_permissions_example.smime"

# Name of the grant section applied to the governance document
CLIENT_GRANT="client_permissions"
RC_GRANT="rc_permissions"

# Expiration date of permissions file
NOT_BEFORE="2025-01-01T00:00:00"
NOT_AFTER="2030-01-01T00:00:00"

# Subject of the private certificate authority
PRIVATE_CA_COUNTRY="JP"
PRIVATE_CA_STATE="Nagano"
PRIVATE_CA_LOCALITY="Azumino"
PRIVATE_CA_ORGANIZATION="Private_Example"
PRIVATE_CA_ORGANIZATIONUNIT="Private_Example"
PRIVATE_CA_COMMONNAME="Private_Example"
PRIVATE_CA_EMAILADDRESS="Private@example.com"

PRIVATE_CA_SUBJECT = (  f"/C={PRIVATE_CA_COUNTRY}"
                        f"/ST={PRIVATE_CA_STATE}"
                        f"/L={PRIVATE_CA_LOCALITY}"
                        f"/O={PRIVATE_CA_ORGANIZATION}"
                        f"/OU={PRIVATE_CA_ORGANIZATIONUNIT}"
                        f"/CN={PRIVATE_CA_COMMONNAME}"
                        f"/emailAddress={PRIVATE_CA_EMAILADDRESS}"  
                    )

# Validity period of the self-signed certificate for the private certificate authority 
PIIVATE_CA_EXPIRATION_DATE="3650"

# Expiration date of the signed certificates for the controller and external PC
USER_EXPIRATION_DATE="1825"

# Subject of the controller and external PC
USER_COUNTRY="JP"
USER_STATE="Nagano"
USER_LOCALITY="Azumino"
USER_ORGANIZATION="User_Example"
USER_ORGANIZATIONUNIT="User_Example"
USER_COMMONNAME="User_Example"
USER_EMAILADDRESS="User@example.com"

USER_SUBJECT = (    f"/C={USER_COUNTRY}"
                    f"/ST={USER_STATE}"
                    f"/L={USER_LOCALITY}"
                    f"/O={USER_ORGANIZATION}"
                    f"/OU={USER_ORGANIZATIONUNIT}"
                    f"/CN={USER_COMMONNAME}/"
                    f"emailAddress={USER_EMAILADDRESS}"
                )

def create_folder(folder_name):
    """Create a folder to store the generated files"""

    os.makedirs(folder_name)

def generate_private_key(private_file_name):
    """ Generate a private key"""

    subprocess.run([
        'openssl', 'genrsa',
        '-out', private_file_name,
        '2048'
    ],check=True)

def generate_csr(cert_key_file_name,csr_file_name):
    """ Generate a certificate signing request file"""

    subprocess.run([
        'openssl', 'req', '-new',
        '-key', cert_key_file_name,
        '-out', csr_file_name,
        '-subj', USER_SUBJECT
    ],check=True)

def generate_cert(csr_file_name, cert_file_name):
    """ Generate a certificate authority"""

    subprocess.run([
        'openssl','x509','-req',
        '-CA', CA_CERT_FILE,
        '-CAkey', CA_CERT_KEY_FILE,
        '-CAcreateserial',
        '-days',USER_EXPIRATION_DATE,
        '-in', csr_file_name,
        '-out', cert_file_name
    ],check=True)


def generate_permissions(permissions_file_name,grant):
    """ Generate a permissions file"""

    permissions_xml_root=ET.Element("dds")
    permissions=ET.SubElement(permissions_xml_root,"permissions")
    grant=ET.SubElement(permissions,"grant", attrib={"name":grant})

    permissions_email=f"emailAddress={USER_EMAILADDRESS}"
    permissions_cn=f"CN={USER_COMMONNAME}"
    permissions_ou=f"OU={USER_ORGANIZATIONUNIT}"
    permissions_o=f"O={USER_ORGANIZATION}"
    permissions_l=f"L={USER_LOCALITY}"
    permissions_st=f"ST={USER_STATE}"
    permissions_c=f"C={USER_COUNTRY}"

    subject_for_permissions = (
        ','.join([permissions_email,permissions_cn, permissions_ou, permissions_o, 
        permissions_l, permissions_st, permissions_c])
    )
    subject_name=ET.SubElement(grant,"subject_name")
    subject_name.text=subject_for_permissions

    validity=ET.SubElement(grant,"validity")
    not_before=ET.SubElement(validity,"not_before")
    not_before.text=NOT_BEFORE
    not_after=ET.SubElement(validity,"not_after")
    not_after.text=NOT_AFTER

    allow_rule=ET.SubElement(grant,"allow_rule")
    dds_domains=ET.SubElement(allow_rule,"domains")
    dds_id_range=ET.SubElement(dds_domains,"id_range")
    permissions_dds_id_min=ET.SubElement(dds_id_range,"min")
    permissions_dds_id_min.text="0"
    permissions_dds_id_max=ET.SubElement(dds_id_range,"max")
    permissions_dds_id_max.text="230"

    publish=ET.SubElement(allow_rule,"publish")
    topics=ET.SubElement(publish,"topics")
    topic=ET.SubElement(topics,"topic")
    topic.text="DDSCommunicationTopic"
    partitions=ET.SubElement(publish,"partitions")
    partition=ET.SubElement(partitions,"partition")
    if permissions_file_name == CLIENT_PERMISSIONS_FILE:
        partition.text="RTMC_CMD"
    if permissions_file_name == RC_PERMISSIONS_FILE:
        partition.text="RTMC_RES"


    subscribe=ET.SubElement(allow_rule,"subscribe")
    topics=ET.SubElement(subscribe,"topics")
    topic=ET.SubElement(topics,"topic")
    topic.text="DDSCommunicationTopic"
    partitions=ET.SubElement(subscribe,"partitions")
    partition=ET.SubElement(partitions,"partition")
    if permissions_file_name == CLIENT_PERMISSIONS_FILE:
        partition.text="RTMC_RES"
    if permissions_file_name == RC_PERMISSIONS_FILE:
        partition.text="RTMC_CMD"

    default=ET.SubElement(grant,"default")
    default.text="DENY"

    dds_permissions_xml_string=ET.tostring(permissions_xml_root,encoding="utf-8")
    dds_permissions_parsed_xml=xml.dom.minidom.parseString(dds_permissions_xml_string)
    dds_permissions_proper_xml=dds_permissions_parsed_xml.toprettyxml(indent=" ")

    with open(permissions_file_name, "w", encoding="utf-8") as dds_permissions_f:
        dds_permissions_f.write(dds_permissions_proper_xml)

if not os.path.exists(GOVERNANCE_FILE):
    governance_xml_root=ET.Element("dds")
    domain_access_rules=ET.SubElement(governance_xml_root,"domain_access_rules")
    domain_rule=ET.SubElement(domain_access_rules,"domain_rule")
    domains=ET.SubElement(domain_rule,"domains")
    id_range=ET.SubElement(domains,"id_range")
    dds_id_min=ET.SubElement(id_range,"min")
    dds_id_min.text="0"
    dds_id_max=ET.SubElement(id_range,"max")
    dds_id_max.text="230"

    allow_unauthenticated_participants=(
        ET.SubElement(domain_rule,"allow_unauthenticated_participants")
    )

    allow_unauthenticated_participants.text="false"

    enable_join_access_control=ET.SubElement(domain_rule,"enable_join_access_control")
    enable_join_access_control.text="true"

    discovery_protection_kind=ET.SubElement(domain_rule,"discovery_protection_kind")
    discovery_protection_kind.text="ENCRYPT"

    liveliness_protection_kind=ET.SubElement(domain_rule,"liveliness_protection_kind")
    liveliness_protection_kind.text="NONE"

    rtps_protection_kind=ET.SubElement(domain_rule,"rtps_protection_kind")
    rtps_protection_kind.text="ENCRYPT"

    topic_access_rules=ET.SubElement(domain_rule,"topic_access_rules")
    topic_rule=ET.SubElement(topic_access_rules,"topic_rule")

    topic_expression=ET.SubElement(topic_rule,"topic_expression")
    topic_expression.text="DDSCommunicationTopic"

    enable_discovery_protection=ET.SubElement(topic_rule,"enable_discovery_protection")
    enable_discovery_protection.text="true"

    enable_liveliness_protection=ET.SubElement(topic_rule,"enable_liveliness_protection")
    enable_liveliness_protection.text="false"

    enable_read_access_control=ET.SubElement(topic_rule,"enable_read_access_control")
    enable_read_access_control.text="true"

    enable_write_access_control=ET.SubElement(topic_rule,"enable_write_access_control")
    enable_write_access_control.text="true"

    metadata_protection_kind=ET.SubElement(topic_rule,"metadata_protection_kind")
    metadata_protection_kind.text="NONE"

    data_protection_kind=ET.SubElement(topic_rule,"data_protection_kind")
    data_protection_kind.text="NONE"

    dds_governance_xml_string=ET.tostring(governance_xml_root,encoding="utf-8")
    dds_governance_parsed_xml=xml.dom.minidom.parseString(dds_governance_xml_string)
    dds_governance_proper_xml=dds_governance_parsed_xml.toprettyxml(indent=" ")

    with open(GOVERNANCE_FILE, "w", encoding="utf-8") as f:
        f.write(dds_governance_proper_xml)

if not os.path.exists(CLIENT_PERMISSIONS_FILE):
    generate_permissions(CLIENT_PERMISSIONS_FILE,CLIENT_GRANT)

if not os.path.exists(RC_PERMISSIONS_FILE):
    generate_permissions(RC_PERMISSIONS_FILE,RC_GRANT)

if not os.path.exists(CA_CERT_FOLDER):
    create_folder(CA_CERT_FOLDER)

if not os.path.exists(CLIENT_CERT_FOLDER):
    create_folder(CLIENT_CERT_FOLDER)

if not os.path.exists(RC_CERT_FOLDER):
    create_folder(RC_CERT_FOLDER)

if not os.path.exists(CA_CERT_KEY_FILE):
    generate_private_key(CA_CERT_KEY_FILE)

if not os.path.exists(CA_CERT_FILE):
    subprocess.run([
        'openssl', 'req', '-x509',
        '-key', CA_CERT_KEY_FILE, 
        '-out', CA_CERT_FILE,
        '-days', PIIVATE_CA_EXPIRATION_DATE,
        '-subj', PRIVATE_CA_SUBJECT
    ],check=True)
    shutil.copy(CA_CERT_FILE,CLIENT_CERT_FOLDER)
    shutil.copy(CA_CERT_FILE,RC_CERT_FOLDER)


if not os.path.exists(CLIENT_CERT_KEY_FILE):
    generate_private_key(CLIENT_CERT_KEY_FILE)

generate_csr(CLIENT_CERT_KEY_FILE,CLIENT_CSR_FILE)
generate_cert(CLIENT_CSR_FILE,CLIENT_CERT_FILE)

if not os.path.exists(RC_CERT_KEY_FILE):
    generate_private_key(RC_CERT_KEY_FILE)

generate_csr(RC_CERT_KEY_FILE,RC_CSR_FILE)
generate_cert(RC_CSR_FILE,RC_CERT_FILE)

if os.path.exists(GOVERNANCE_FILE):
    subprocess.run([
        'openssl','smime','-sign',
        '-in', GOVERNANCE_FILE,
        '-text', '-out', CLIENT_GOVERNANCE_SMIME,
        '-signer',CA_CERT_FILE,
        '-inkey',CA_CERT_KEY_FILE,
    ],check=True)

    subprocess.run([
        'openssl','smime','-sign',
        '-in', GOVERNANCE_FILE,
        '-text', '-out', RC_GOVERNANCE_SMIME,
        '-signer',CA_CERT_FILE,
        '-inkey',CA_CERT_KEY_FILE,
    ],check=True)

if os.path.exists(CLIENT_PERMISSIONS_FILE):
    subprocess.run([
        'openssl','smime','-sign',
        '-in', CLIENT_PERMISSIONS_FILE,
        '-text', '-out', CLIENT_PERMISSIONS_SMIME,
        '-signer',CA_CERT_FILE,
        '-inkey',CA_CERT_KEY_FILE,
    ],check=True)

if os.path.exists(RC_PERMISSIONS_FILE):
    subprocess.run([
        'openssl','smime','-sign',
        '-in', RC_PERMISSIONS_FILE,
        '-text', '-out', RC_PERMISSIONS_SMIME,
        '-signer',CA_CERT_FILE,
        '-inkey',CA_CERT_KEY_FILE,
    ],check=True)
